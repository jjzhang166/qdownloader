#include "fpicdownloader.h"
#include "ui_fpicdownloader.h"

#include <QCloseEvent>

#include "kutil/kcommon.h"
#include "kutil/misc.h"
#include "kutil/widget_helper.h"

FPicDownloader::FPicDownloader(QWidget* parent)
    :QWidget(parent)
{
    ui->setupUi(this);    

    downloader_.setParalledDownload(5);
    connect(&downloader_, &Downloader::finished, [&](int n){
        if (downloader_.isAllDone()){
            ui->btn_go_->stopCountdown();
            emit allDone();
        }
        ui->progress_->setValue(n);
    });
    connect(&downloader_, &Downloader::downloadEvent, this, &FPicDownloader::onDownloadPicture);
    connect(this, &FPicDownloader::closing,
        KPictureDownloaderMan::instance(), &KPictureDownloaderMan::onDlgClosing);
}

FPicDownloader::~FPicDownloader()
{
}

int FPicDownloader::urlRow(const QString& url) const
{
    return downloader_.index(url);
}

void FPicDownloader::closeEvent(QCloseEvent *event)
{
    if (downloader_.isAllDone()){
        if (!KDLG::confirm(QStringLiteral("图片未完全下载，确定退出？"))){
            event->ignore();
            return;
        }
    }
    emit closing(this);
}

void FPicDownloader::setParams( const QString& default_save_path, 
    const QString& title, const QStringList& pics, const QString& url_referer)
{
    downloader_.setReferer(url_referer);

    // 设置默认的存储位置
    ui->path_->setPathMode(QPathEdit::ExistingFolder);
    ui->path_->setPath(default_save_path);

    // 倒计时直接5分钟，不能多次导出
    ui->btn_go_->setCountdown(5 * 60);
    setWindowTitle(title);

    // 下载器设置，内部会修改连接地址
    auto& items = downloader_.setItems(pics);
    ui->progress_->setRange(0, items.count());
    ui->progress_->setValue(0);

    kutil::widget::KTableHelper::InitTableWidget(ui->tbl_pics_, {
        { QStringLiteral("图片地址"), "url", 100, 0, QHeaderView::Stretch },
        { QStringLiteral("状态"), "status", 80, 0, QHeaderView::Fixed }
    }, "url", QSize(100, 100));

    QTableWidget* t = ui->tbl_pics_;
    int i = 0;
    for (auto &s : items){
        t->insertRow(i);
        t->setItem(i, 0, new QTableWidgetItem(s));
        t->setItem(i, 1, new QTableWidgetItem(QStringLiteral("等待……")));
        i++;
    }
}

void FPicDownloader::hideControls()
{
    ui->btn_go_->hide();
    ui->path_tip_->hide();
    ui->path_->hide();
    ui->path_layout_->setGeometry(QRect(0, 0, 0, 0));
    ui->btn_layout_->setGeometry(QRect(0, 0, 0, 0));
}

void FPicDownloader::startDownload()
{
    on_btn_go__clicked();
}

void FPicDownloader::on_btn_go__clicked()
{
    downloader_.start();
}

void FPicDownloader::onDownloadPicture(int evt, QUrl url, int progress, QString err)
{
    int r = urlRow(url.toString());
    if (-1 != r){
        QTableWidget* t = ui->tbl_pics_;
        t->item(r, 1)->setText(err);
    }
}

void FPicDownloader::on_path__pathChanged(QString path)
{
    if (!path.isEmpty()){
        downloader_.setDownloadPath(path);
        KPictureDownloaderMan::instance()->setLastSavePath(path);
    }
}

FPicDownloader* KPictureDownloaderMan::download(const QString&url, const QString& title,
    const QStringList& pics, const QString& url_referer)
{
    if (FPicDownloader* p = workers_.value(url)){
        p->raise();
        return p;
    }
    else{
        p = new FPicDownloader(nullptr);
        p->setParams(last_save_path_, title, pics, url_referer);
        p->setAttribute(Qt::WA_DeleteOnClose);
        p->show();
        workers_.insert(url, p);
        return p;
    }
}

void KPictureDownloaderMan::onDlgClosing(FPicDownloader* p)
{
    QString url = workers_.key(p);
    if (!url.isEmpty()){
        workers_.remove(url);
    }
}


