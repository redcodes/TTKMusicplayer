#include "musicrightareawidget.h"
#include "ui_musicapplication.h"
#include "musicuiobject.h"
#include "musiclrccontainerfordesktop.h"
#include "musicvideoplayer.h"

MusicRightAreaWidget::MusicRightAreaWidget(QWidget *parent)
    : QWidget(parent), m_videoPlayer(NULL)
{
    m_supperClass = parent;
    m_musiclrcfordesktop = new MusicLrcContainerForDesktop(parent);
}

MusicRightAreaWidget::~MusicRightAreaWidget()
{
    delete m_videoPlayer;
    delete m_musiclrcfordesktop;
}

void MusicRightAreaWidget::setupUi(Ui::MusicApplication* ui)
{
    m_ui = ui;

    ui->musicSearchRefreshButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->musicSearchRefreshButton->setStyleSheet(MusicUIObject::MPushButtonStyle03);
    ui->musicSearchRefreshButton->setIconSize(QSize(25,25));
    ui->musicSearchRefreshButton->setIcon(QIcon(QString::fromUtf8(":/image/flash")));
    connect(ui->musicSearchRefreshButton,SIGNAL(clicked()), SLOT(musicSearchRefreshButtonRefreshed()));

    ui->musicIndexWidgetButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->musicIndexWidgetButton->setStyleSheet(MusicUIObject::MPushButtonStyle03);
    connect(ui->musicIndexWidgetButton,SIGNAL(clicked()), SLOT(musicIndexWidgetButtonSearched()));

    ui->musicSearchWidgetButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->musicSearchWidgetButton->setStyleSheet(MusicUIObject::MPushButtonStyle03);
    connect(ui->musicSearchWidgetButton,SIGNAL(clicked()), SLOT(musicSearchWidgetButtonSearched()));

    ui->musicLrcWidgetButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->musicLrcWidgetButton->setStyleSheet(MusicUIObject::MPushButtonStyle03);
    connect(ui->musicLrcWidgetButton,SIGNAL(clicked()), SLOT(musicLrcWidgetButtonSearched()));

    ui->vedioWidgetButton->setCursor(QCursor(Qt::PointingHandCursor));
    ui->vedioWidgetButton->setStyleSheet(MusicUIObject::MPushButtonStyle03);
    connect(ui->vedioWidgetButton,SIGNAL(clicked()), SLOT(musicVedioWidgetButtonSearched()));
    ///////////////////////////////////////////////////////
    connect(m_musiclrcfordesktop,SIGNAL(theCurrentLrcUpdated()), m_supperClass,
                 SLOT(musicCurrentLrcUpdated()));
    connect(m_musiclrcfordesktop,SIGNAL(changeCurrentLrcColorSetting()), m_supperClass,
                 SLOT(musicSetting()));
    connect(m_musiclrcfordesktop,SIGNAL(desktopLrcClosed()), m_supperClass,
                 SLOT(desktopLrcClosed()));
    connect(m_musiclrcfordesktop,SIGNAL(changeCurrentLrcColorCustom()), m_supperClass,
                 SLOT(changeDesktopLrcWidget()));
    connect(m_musiclrcfordesktop,SIGNAL(setWindowLockedChanged(bool)), m_supperClass,
                 SLOT(lockDesktopLrc(bool)));
    ///////////////////////////////////////////////////////
    connect(ui->musiclrccontainerforinline,SIGNAL(changeCurrentLrcColorCustom()), m_supperClass,
                 SLOT(changeInlineLrcWidget()));
    connect(ui->musiclrccontainerforinline,SIGNAL(theCurrentLrcUpdated()), m_supperClass,
                 SLOT(musicCurrentLrcUpdated()));
    connect(ui->musiclrccontainerforinline, SIGNAL(theArtBgHasChanged()),
                 SIGNAL(updateBgThemeDownload()));
    connect(ui->musiclrccontainerforinline,SIGNAL(changeCurrentLrcColorSetting()), m_supperClass,
                 SLOT(musicSetting()));
    connect(ui->musiclrccontainerforinline,SIGNAL(updateCurrentTime(qint64)), m_supperClass,
                 SLOT(updateCurrentTime(qint64)));
}


void MusicRightAreaWidget::stopLrcMask()
{
    if( checkSettingParameterValue() )
    {
       m_ui->musiclrccontainerforinline->stopLrcMask();
       m_musiclrcfordesktop->stopLrcMask();
    }
}

void MusicRightAreaWidget::startTimerClock()
{
    if( checkSettingParameterValue() )
    {
       m_ui->musiclrccontainerforinline->startTimerClock();
       m_musiclrcfordesktop->startTimerClock();
    }
}

void MusicRightAreaWidget::showPlayStatus(bool status)
{
    m_musiclrcfordesktop->showPlayStatus(status);
}

void MusicRightAreaWidget::setDestopLrcVisible(const QString& status)
{
    setDestopLrcVisible( status == "true" ? true : false);
}

bool MusicRightAreaWidget::getDestopLrcVisible()
{
    return m_musiclrcfordesktop->isVisible();
}

void MusicRightAreaWidget::setInlineLrcVisible(const QString& status)
{
    m_ui->musiclrccontainerforinline->setVisible(status == "true" ? true : false);
}

void MusicRightAreaWidget::setSettingParameter()
{
    m_musiclrcfordesktop->setSettingParameter();
    m_ui->musiclrccontainerforinline->setSettingParameter();
}

bool MusicRightAreaWidget::checkSettingParameterValue()
{
    return ( M_SETTING.value(MusicSettingManager::ShowInlineLrcChoiced).toBool() ||
             M_SETTING.value(MusicSettingManager::ShowDesktopLrcChoiced).toBool() )
             ? true : false;
}

void MusicRightAreaWidget::updateCurrentLrc(qint64 current, qint64 total, bool playStatus)
{
    m_ui->musiclrccontainerforinline->setCurrentPosition(current);
    //Direct access to the audio file is the total time, in milliseconds
    MIntStringMap lrcContainer(m_ui->musiclrccontainerforinline->getLrcContainer());
    //The corresponding access to current time lyrics
    if(!lrcContainer.isEmpty())
    {
        //After get the current time in the lyrics of the two time points
        qint64 previous = 0;
        qint64 later = 0;
        //Keys () method returns a list of lrcContainer
        foreach (qint64 value, lrcContainer.keys())
        {
            if(current >= value)
                previous = value;
            else
            {
                later = value;
                break;
            }
        }
        //To the last line, set the later to song total time value
        if(later == 0) later = total;
        //The lyrics content corresponds to obtain the current time
        QString currentLrc = lrcContainer.value(previous);
        QString laterLrc = lrcContainer.value(later);

        //If this is a new line of the lyrics, then restart lyrics display mask
        if(currentLrc != m_ui->musiclrccontainerforinline->text())
        {
            qint64 intervalTime = later - previous;
            if(!playStatus)
            {
                m_ui->musiclrccontainerforinline->updateCurrentLrc(intervalTime);
            }
            m_musiclrcfordesktop->updateCurrentLrc(currentLrc, laterLrc, intervalTime);
        }
    }
}

void MusicRightAreaWidget::loadCurrentSongLrc(const QString& name, const QString &path)
{
    if( checkSettingParameterValue() )
    {
        m_ui->musiclrccontainerforinline->stopLrcMask();
        m_ui->musiclrccontainerforinline->setCurrentSongName( name );
        m_ui->musiclrccontainerforinline->transLrcFileToTime( path.trimmed() );
        m_musiclrcfordesktop->setCurrentSongName( name );
        m_musiclrcfordesktop->initCurrentLrc();
    }
}

void MusicRightAreaWidget::setSongSpeedAndSlow(qint64 time)
{
    m_ui->musiclrccontainerforinline->setSongSpeedAndSlow(time);
}

void MusicRightAreaWidget::getParameterSetting()
{
    setSettingParameter();
    if(M_SETTING.value(MusicSettingManager::ShowInlineLrcChoiced).toBool())
        m_ui->musiclrccontainerforinline->show();
    else
        m_ui->musiclrccontainerforinline->close();
    bool showDeskLrc = M_SETTING.value(MusicSettingManager::ShowDesktopLrcChoiced).toBool();
    m_musiclrcfordesktop->setVisible(showDeskLrc);
    m_ui->musicDesktopLrc->setChecked(showDeskLrc);
}

void MusicRightAreaWidget::setDestopLrcVisible(bool v)
{
    m_ui->musicDesktopLrc->setChecked(v);
    m_musiclrcfordesktop->setVisible(v);
}

void MusicRightAreaWidget::setWindowLockedChanged()
{
    m_musiclrcfordesktop->setWindowLockedChanged();
}

void MusicRightAreaWidget::musicSearchButtonSearched()
{
    QString searchedQString = m_ui->musicSongSearchLine->text().trimmed();
    //The string searched wouldn't allow to be none
    if( !searchedQString.isEmpty())
    {
        m_ui->SurfaceStackedWidget->setCurrentIndex(1);
        createVedioWidget(false);
        m_ui->songSearchWidget->startSearchQuery(searchedQString);
    }
}

void MusicRightAreaWidget::musicIndexWidgetButtonSearched()
{
    //Show the first index of widget
    m_ui->SurfaceStackedWidget->setCurrentIndex(0);
    createVedioWidget(false);
}

void MusicRightAreaWidget::musicSearchWidgetButtonSearched()
{
    //Show searched song lists
    m_ui->SurfaceStackedWidget->setCurrentIndex(1);
    createVedioWidget(false);
}

void MusicRightAreaWidget::musicLrcWidgetButtonSearched()
{
    //Show lrc display widget
    m_ui->SurfaceStackedWidget->setCurrentIndex(2);
    createVedioWidget(false);
    emit updateBgThemeDownload();
}

void MusicRightAreaWidget::musicSearchRefreshButtonRefreshed()
{
    createVedioWidget(false);
    //Refresh the search music song
    musicSearchButtonSearched();
}

void MusicRightAreaWidget::createVedioWidget(bool create)
{
    if(create)
    {
        delete m_videoPlayer;
        m_videoPlayer = new MusicVideoPlayer;
        m_ui->SurfaceStackedWidget->addWidget(m_videoPlayer);
//        if(!m_playControl) musicKey();
    }
    else if(m_videoPlayer)
    {
        m_ui->SurfaceStackedWidget->removeWidget(m_videoPlayer);
        delete m_videoPlayer;
        m_videoPlayer = NULL;
    }
    emit updateBackgroundTheme();
}

void MusicRightAreaWidget::musicVedioWidgetButtonSearched()
{
    createVedioWidget(true);
    m_ui->SurfaceStackedWidget->setCurrentIndex(3);\
}

void MusicRightAreaWidget::musicVedioWidgetButtonDoubleClicked()
{
//    m_ui->SurfaceStackedWidget->removeWidget(m_videoPlayer);
//    m_videoPlayer->showFullScreen();
}