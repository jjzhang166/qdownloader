#pragma once

#include <QWidget>
#include <QDir>
#include "downloader.h"
#include "kutil/singleton.h"

namespace Ui {
    class FPicDownloader;
}

namespace qdownloader 
{

	class KPictureDownloaderMan;
	class FPicDownloader : public QWidget
	{
		Q_OBJECT

			friend class KPictureDownloaderMan;

	private:
		FPicDownloader(QWidget* parent);

		~FPicDownloader();
		int urlRow(const QString& url)const;
		void downloadUrl(const QString& url)const;

		virtual void closeEvent(QCloseEvent *event) override;

	public:
		void setParams(const QString& default_save_path, const QString& title,
			const QStringList& pics);
		void hideControls();
		void startDownload();

		protected slots:
		void on_btn_go__clicked();
		void onDownloadPicture(int evt, QUrl url, int progress, QString err);
		void on_path__pathChanged(QString path);

	signals:
		void closing(FPicDownloader*);
		// 图片下载了多少个了
		void allDone();

	private:
		Ui::FPicDownloader* ui = nullptr;
		Downloader downloader_;
	};

	class KPictureDownloaderMan : public SingletonWithBase<KPictureDownloaderMan, QObject>
	{
		Q_OBJECT

			friend class SingletonWithBase<KPictureDownloaderMan, QObject>;

	private:
		KPictureDownloaderMan()
			:SingletonWithBase<KPictureDownloaderMan, QObject>(nullptr) {
			last_save_path_ = QDir::currentPath() + "/pictures/";
			QDir().mkdir(last_save_path_);
		}
		~KPictureDownloaderMan() {

		}

	public:
		FPicDownloader* download(const QString&url, const QString& title,
			const QStringList& pics);
		void setLastSavePath(const QString& path) {
			last_save_path_ = path;
		}

		inline QString lastSavePath()const {
			return last_save_path_;
		}

		public slots:
		void onDlgClosing(FPicDownloader* p);

	private:
		QHash<QString, FPicDownloader*> workers_;
		// 图片的保存位置
		QString last_save_path_;
	};


}
