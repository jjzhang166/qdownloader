#include "downloader.h"
#include <QFileInfo>
#include <QDir>

#include "knet/net.h"
#include "kutil/misc.h"

Downloader::Downloader(QObject *parent)
    : QObject(parent)
{
}

Downloader::~Downloader()
{
}

const QStringList& Downloader::setItems(const QStringList& items)
{
    items_ = items;
    items_.removeDuplicates();
    started_ = false;
    idx_ = 0;
    done_ = 0;

    for (QString& s : items_){
        s = QUrl(s).toString(); // ��һ��ת�������ܻ������ֽ�����
    }
    return items_;
}

void Downloader::start()
{
    if (!started_){
        started_ = true;
        for (idx_ = 0; (idx_ < items_.count()) && (idx_ < parell_download_); ++idx_){
            download(items_.at(idx_));
        }
    }
    // Ҫָ�����һ�����ص�λ��
    idx_--;
}

void Downloader::stop()
{
    for (auto& s : items_){
        net::KHttpCancelDownload(QUrl(s), this);
    }
    started_ = false;
}

void Downloader::onDownload(int evt, QUrl url, int progress, QNetworkReply::NetworkError)
{
    bool download_next = true;
    QString err;
    switch (evt)
    {
    case net::HttpDownload_Event_Finished:
        err = QStringLiteral("�����");
        done_++;
        break;

    case net::HttpDownload_Event_Error:
        err = QStringLiteral("����%1").arg(err);
        done_++;
        break;

    case net::HttpDownload_Event_Progress:
        download_next = false;
        err = QStringLiteral("%1%").arg(progress);
        break;
    }

    emit downloadEvent(evt, url, progress, err);
    if (download_next){
        // ��ǰ��������ɵ�
        emit finished(done_);

        if (idx_ < items_.count() - 1){ // idx ������
            idx_++;
            download(items_[idx_]); // ������ɵݹ����
        }
    }
}

void Downloader::download(const QString& surl)
{
    QUrl url(surl);
    QString save_file = save_dir_ + "/" + kutil::url2Filename(surl);
    if (QDir().exists(save_file)){
        // ������ļ��Ѿ����ڣ���Ҫ�����ˣ�ֱ��֪ͨ���������
        onDownload(net::HttpDownload_Event_Finished, url, 100, QNetworkReply::NoError);
    }
    else{
        QString referer = referer_.isEmpty() ? url.host() : referer_;
        const QHash<QString, QString> headers = {
            { "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36" },
            { "accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" },
            { "Referer", referer }
        };
        net::KHttpDownload(url, save_file, this,
            SLOT(onDownload(int, QUrl, int, QNetworkReply::NetworkError)), headers);
    }
}
