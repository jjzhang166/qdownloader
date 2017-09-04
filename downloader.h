#pragma once

#include <QObject>
#include <QNetworkReply>

namespace qdownloader 
{

	class Downloader : public QObject
	{
		Q_OBJECT

	public:
		Downloader(QObject *parent = nullptr);
		~Downloader();

		// 用于文件下载防盗链
		void setReferer(const QString& refer) {
			referer_ = refer;
		}

		void setParalledDownload(int n) {
			parell_download_ = n;
		}

		void setDownloadPath(const QString& path) {
			save_dir_ = path;
		}

		const QStringList& setItems(const QStringList& items);

		int index(const QString& url)const {
			return items_.indexOf(url);
		}

		void start();
		void stop();

		bool isAllDone()const {
			return done_ >= items_.count();
		}

	protected:
		void download(const QString& url);

	signals:
		void downloadEvent(int evt, QUrl, int progress, QString error);
		void finished(int);

		protected slots:
		void onDownload(int evt, QUrl, int progress, QNetworkReply::NetworkError);

	private:
		// 保存到的路径
		QString save_dir_;
		// 所有需要下载的连接
		QStringList items_;
		// 文件防盗链
		QString referer_;

		// 当前最后一个下载的位置
		int idx_ = 0;
		// 当前已确定下载完成/下载错误的图片数量
		int done_ = 0;
		// 同时下载的个数
		int parell_download_ = 5;

		bool started_ = false;
	};

}
