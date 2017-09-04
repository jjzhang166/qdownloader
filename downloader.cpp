#include "downloader.h"
#include <QFileInfo>
#include <QDir>

#include "knet/net.h"
#include "kutil/misc.h"

namespace qdownloader
{

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

		for (QString& s : items_) {
			s = QUrl(s).toString(); // 做一次转换，可能会有名字解析等
		}
		return items_;
	}

	void Downloader::start()
	{
		if (!started_) {
			started_ = true;
			for (idx_ = 0; (idx_ < items_.count()) && (idx_ < parell_download_); ++idx_) {
				download(items_.at(idx_));
			}
		}
		// 要指向最后一个下载的位置
		idx_--;
	}

	void Downloader::stop()
	{
		for (auto& s : items_) {
			net::KHttpCancelDownload(QUrl(s), this);
		}
		started_ = false;
	}

	void Downloader::onDownload(int evt, QUrl url, int progress, QNetworkReply::NetworkError err)
	{
		bool download_next = true;
		QString s_error;
		switch (evt)
		{
		case net::HttpDownload_Event_Finished:
			s_error = QStringLiteral("已完成");
			done_++;
			break;

		case net::HttpDownload_Event_Error:
			s_error = QStringLiteral("错误：%1").arg(err);
			done_++;
			break;

		case net::HttpDownload_Event_Progress:
			download_next = false;
			s_error = QStringLiteral("%1%").arg(progress);
			break;
		}

		emit downloadEvent(evt, url, progress, s_error);
		if (download_next) {
			// 当前已下载完成的
			emit finished(done_);

			if (idx_ < items_.count() - 1) { // idx 是索引
				idx_++;
				download(items_[idx_]); // 可能造成递归调用
			}
		}
	}

	QString Downloader::download(const QString& surl)
	{
		QUrl url(surl);
		QString save_file = save_dir_ + "/" + kutil::url2Filename(surl);
		if (QDir().exists(save_file)) {
			// 如果此文件已经存在，则不要下载了，直接通知下载完成了
			onDownload(net::HttpDownload_Event_Finished, url, 100, QNetworkReply::NoError);
		}
		else {
			const QHash<QString, QString> headers = {
				{ "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36" },
				{ "accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8" },
				{ "Referer", url.host() }
			};
			net::KHttpDownload(url, save_file, this,
				SLOT(onDownload(int, QUrl, int, QNetworkReply::NetworkError)), headers);
		}
		return save_file;
	}

}
