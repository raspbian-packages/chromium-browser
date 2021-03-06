// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/chrome_cleaner/zip_archiver/sandboxed_zip_archiver.h"

#include <utility>

#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/waitable_event.h"
#include "base/win/scoped_handle.h"
#include "chrome/chrome_cleaner/os/disk_util.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace chrome_cleaner {

namespace {

using mojom::ZipArchiverResultCode;

constexpr wchar_t kDefaultFileStreamSuffix[] = L"::$DATA";
constexpr uint32_t kMinimizedReadAccess =
    SYNCHRONIZE | FILE_READ_DATA | FILE_READ_ATTRIBUTES;
constexpr uint32_t kMinimizedWriteAccess =
    SYNCHRONIZE | FILE_WRITE_DATA | FILE_READ_ATTRIBUTES;

// NTFS file stream can be specified by appending ":" to the filename. We remove
// the default file stream "::$DATA" so it won't break the filename in the
// following uses. For other file streams, we don't archive and ignore them.
bool GetSanitizedFileName(const base::FilePath& path,
                          base::string16* output_sanitized_filename) {
  DCHECK(output_sanitized_filename);

  base::string16 sanitized_filename = path.BaseName().AsUTF16Unsafe();
  if (base::EndsWith(sanitized_filename, kDefaultFileStreamSuffix,
                     base::CompareCase::INSENSITIVE_ASCII)) {
    // Remove the default file stream suffix.
    sanitized_filename.erase(
        sanitized_filename.end() - wcslen(kDefaultFileStreamSuffix),
        sanitized_filename.end());
  }
  // If there is any ":" in |sanitized_filename|, it either points to a
  // non-default file stream or is abnormal. Don't archive in this case.
  if (sanitized_filename.find(L":") != base::string16::npos)
    return false;

  *output_sanitized_filename = sanitized_filename;
  return true;
}

void RunArchiver(mojom::ZipArchiverPtr* zip_archiver_ptr,
                 mojo::ScopedHandle mojo_src_handle,
                 mojo::ScopedHandle mojo_zip_handle,
                 const std::string& filename,
                 const std::string& password,
                 mojom::ZipArchiver::ArchiveCallback callback) {
  DCHECK(zip_archiver_ptr);

  (*zip_archiver_ptr)
      ->Archive(std::move(mojo_src_handle), std::move(mojo_zip_handle),
                filename, password, std::move(callback));
}

void OnArchiveDone(ZipArchiverResultCode* return_code,
                   base::WaitableEvent* waitable_event,
                   ZipArchiverResultCode result_code) {
  *return_code = static_cast<ZipArchiverResultCode>(result_code);
  waitable_event->Signal();
}

}  // namespace

SandboxedZipArchiver::SandboxedZipArchiver(
    scoped_refptr<MojoTaskRunner> mojo_task_runner,
    UniqueZipArchiverPtr zip_archiver_ptr,
    const base::FilePath& dst_archive_folder,
    const std::string& zip_password)
    : mojo_task_runner_(mojo_task_runner),
      zip_archiver_ptr_(std::move(zip_archiver_ptr)),
      dst_archive_folder_(dst_archive_folder),
      zip_password_(zip_password) {
  // Make sure the |zip_archiver_ptr| is bound with the |mojo_task_runner|.
  DCHECK(zip_archiver_ptr_.get_deleter().task_runner_ == mojo_task_runner);
}

SandboxedZipArchiver::~SandboxedZipArchiver() = default;

// |SandboxedZipArchiver::Archive| archives the source file into a
// password-protected zip file stored in the |dst_archive_folder|. The format of
// zip file name is "|basename of the source file|_|hexdigest of the source file
// hash|.zip".
ZipArchiverResultCode SandboxedZipArchiver::Archive(
    const base::FilePath& src_file_path,
    base::FilePath* output_zip_file_path) {
  DCHECK(output_zip_file_path);

  // Open the source file with minimized rights for reading.
  // Without |FILE_SHARE_WRITE| and |FILE_SHARE_DELETE|, |src_file_path| cannot
  // be manipulated or replaced until |DoArchive| returns. This prevents the
  // following checks from TOCTTOU. Because |base::IsLink| doesn't work on
  // Windows, use |FILE_FLAG_OPEN_REPARSE_POINT| to open a symbolic link then
  // check. To eliminate any TOCTTOU, use |FILE_FLAG_BACKUP_SEMANTICS| to open a
  // directory then check.
  base::File src_file(::CreateFile(
      src_file_path.AsUTF16Unsafe().c_str(), kMinimizedReadAccess,
      FILE_SHARE_READ, nullptr, OPEN_EXISTING,
      FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
  if (!src_file.IsValid()) {
    LOG(ERROR) << "Unable to open the source file.";
    return ZipArchiverResultCode::kErrorCannotOpenSourceFile;
  }

  BY_HANDLE_FILE_INFORMATION src_file_info;
  if (!::GetFileInformationByHandle(src_file.GetPlatformFile(),
                                    &src_file_info)) {
    LOG(ERROR) << "Unable to get the source file information.";
    return ZipArchiverResultCode::kErrorIO;
  }

  // Don't archive symbolic links.
  if (src_file_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    return ZipArchiverResultCode::kIgnoredSourceFile;

  // Don't archive directories. And |ZipArchiver| shouldn't get called with a
  // directory path.
  if (src_file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    LOG(ERROR) << "Tried to archive a directory.";
    return ZipArchiverResultCode::kIgnoredSourceFile;
  }

  base::string16 sanitized_src_filename;
  if (!GetSanitizedFileName(src_file_path, &sanitized_src_filename))
    return ZipArchiverResultCode::kIgnoredSourceFile;

  // TODO(veranika): Check the source file size once the limit is determined.

  std::string src_file_hash;
  if (!ComputeSHA256DigestOfPath(src_file_path, &src_file_hash)) {
    LOG(ERROR) << "Unable to hash the source file.";
    return ZipArchiverResultCode::kErrorIO;
  }

  // Zip file name format: "|source basename|_|src_file_hash|.zip"
  const base::FilePath zip_filename(
      base::StrCat({sanitized_src_filename, L"_",
                    base::UTF8ToUTF16(src_file_hash), L".zip"}));
  const base::FilePath zip_file_path = dst_archive_folder_.Append(zip_filename);

  // Fail if the zip file exists.
  if (base::PathExists(zip_file_path))
    return ZipArchiverResultCode::kZipFileExists;

  // Create and open the zip file with minimized rights for writing.
  base::File zip_file(::CreateFile(zip_file_path.AsUTF16Unsafe().c_str(),
                                   kMinimizedWriteAccess, 0, nullptr,
                                   CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
  if (!zip_file.IsValid()) {
    LOG(ERROR) << "Unable to create the zip file.";
    return ZipArchiverResultCode::kErrorCannotCreateZipFile;
  }

  const std::string filename_in_zip = base::UTF16ToUTF8(sanitized_src_filename);
  ZipArchiverResultCode result_code =
      DoArchive(std::move(src_file), std::move(zip_file), filename_in_zip);
  if (result_code != ZipArchiverResultCode::kSuccess) {
    // The |zip_file| has been closed when returned from the scope of
    // |DoArchive|. Delete the incomplete zip file directly.
    if (!base::DeleteFile(zip_file_path, /*recursive=*/false))
      LOG(ERROR) << "Failed to delete the incomplete zip file.";

    return result_code;
  }

  *output_zip_file_path = zip_file_path;
  return ZipArchiverResultCode::kSuccess;
}

ZipArchiverResultCode SandboxedZipArchiver::DoArchive(
    base::File src_file,
    base::File zip_file,
    const std::string& filename_in_zip) {
  ZipArchiverResultCode result_code;
  base::WaitableEvent waitable_event(
      base::WaitableEvent::ResetPolicy::MANUAL,
      base::WaitableEvent::InitialState::NOT_SIGNALED);

  // Unretained pointer of |zip_archiver_ptr_| is safe because its deleter
  // is run on the same task runner. If |zip_archiver_ptr_| is destructed later,
  // the deleter will be scheduled after this task.
  mojo_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(
          RunArchiver, base::Unretained(zip_archiver_ptr_.get()),
          mojo::WrapPlatformFile(src_file.TakePlatformFile()),
          mojo::WrapPlatformFile(zip_file.TakePlatformFile()), filename_in_zip,
          zip_password_,
          base::BindOnce(OnArchiveDone, &result_code, &waitable_event)));

  waitable_event.Wait();
  return result_code;
}

ResultCode SpawnZipArchiverSandbox(
    const base::FilePath& dst_archive_folder,
    const std::string& zip_password,
    scoped_refptr<MojoTaskRunner> mojo_task_runner,
    base::OnceClosure connection_error_handler,
    std::unique_ptr<SandboxedZipArchiver>* sandboxed_zip_archiver) {
  ZipArchiverSandboxSetupHooks setup_hooks(mojo_task_runner,
                                           std::move(connection_error_handler));
  DCHECK(sandboxed_zip_archiver);

  ResultCode result_code =
      SpawnSandbox(&setup_hooks, SandboxType::kZipArchiver);
  if (result_code == RESULT_CODE_SUCCESS) {
    *sandboxed_zip_archiver = std::make_unique<SandboxedZipArchiver>(
        mojo_task_runner, setup_hooks.TakeZipArchiverPtr(), dst_archive_folder,
        zip_password);
  }

  return result_code;
}

}  // namespace chrome_cleaner
