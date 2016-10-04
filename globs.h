#ifndef GLOBS_H
#define GLOBS_H

enum DownloadState {
    DOWNLOAD_FINISHED = 101,
    DOWNLOAD_ERROR,
    DOWNLOAD_STARTED = -1,
    DOWNLOAD_QUEUED = -2,
    DOWNLOAD_READY = -3,
    DOWNLOAD_NA = -4
};

#endif // GLOBS_H
