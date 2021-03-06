#include "tracking/particlefilter/ParticleFilterWidgetGui.h"
#include "ui_ParticleFilterWidgetGui.h"

ParticleFilterWidgetGui::ParticleFilterWidgetGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParticleFilterWidgetGui)
{
    m_VideoPlayer = new VideoPlayer(this);
    QObject::connect(m_VideoPlayer, SIGNAL(playerFrame(QImage)), SLOT(updatePlayerUI(QImage)));
    ui->setupUi(this);
    ui->start_pushButton->setEnabled(false);
    ui->horizontalSlider->setEnabled(false);
    ui->particlesToDisplaySlider->setEnabled(false);
    m_particleCount = ui->particleCountSpinBox->value();
    m_numIters = ui->iterCountSpinBox->value();
    m_particleWidth = ui->particleWidthLSpinBox->value();
    m_particleHeight = ui->particleHeightLSpinBox->value();
    m_histSize = ui->histogramSizespinBox->value();
    ui->particlesToDisplaySlider->setMaximum(m_particleCount);
    ui->histogramSizespinBox->setValue(m_histSize);
    m_RubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

void ParticleFilterWidgetGui::mouseMoveEvent(QMouseEvent *event)
{
    if (m_VideoLodaded)
        m_RubberBand->setGeometry(QRect(m_Point, event->pos()).normalized());
}

void ParticleFilterWidgetGui::mousePressEvent(QMouseEvent *event)
{
    if (m_VideoLodaded)
    {
        m_VideoPlayer->stopVideo();
        if (m_RubberBand->isEnabled())
            m_RubberBand->hide();
        if (ui->display_label->underMouse())
        {
            m_Point = event->pos();
            m_RubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            m_RubberBand->show();
        }
    }
}

void ParticleFilterWidgetGui::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_VideoLodaded)
    {
        QPoint a = mapToGlobal(m_Point);
        QPoint b = event->globalPos();
        a = ui->display_label->mapFromGlobal(a);
        b = ui->display_label->mapFromGlobal(b);
        QPixmap OriginalPix(*ui->display_label->pixmap());
        double sx = ui->display_label->rect().width();
        double sy = ui->display_label->rect().height();
        sx = OriginalPix.width() / sx;
        sy = OriginalPix.height() / sy;
        a = QPoint(int(a.x() * sx), int(a.y() * sy));
        b = QPoint(int(b.x() * sx), int(b.y() * sy));
        m_TargetROI = QRect(a, b);
    }
}

void ParticleFilterWidgetGui::onActionSaveTarget()
{
    m_RubberBand->hide();
    Target *currTarget;
    QPixmap OriginalPix(*ui->display_label->pixmap());
    QPixmap targetPixMap = OriginalPix.copy(m_TargetROI);
    QString labelno = QString::number(m_TargetCount).rightJustified(4, '0');
    bool isOkPressed{};
    QString labelName = "Human_" + labelno;
    QMessageBox msgBox;
    msgBox.setText("Would you like to track this target?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setIconPixmap(targetPixMap);
    int ret = msgBox.exec();
    switch (ret)
    {
    case QMessageBox::Save:
        labelName = QInputDialog::getText(&msgBox, "Set Label Name", " Label :", QLineEdit::Normal, labelName, &isOkPressed);
        if (isOkPressed)
        {
            currTarget = new Target(labelName, targetPixMap.toImage(), m_histSize);
            m_TargetsVector.push_back(currTarget);
            ui->targets_ListWidget->addItem(currTarget->getLabel());
            m_TargetCount = m_TargetsVector.size();
            break;
        }
        else
           break;
    case QMessageBox::Cancel:
    default:
        break;
    }
}

void ParticleFilterWidgetGui::onActionSetupPF()
{
    std::tuple<int, int> frameSize = m_VideoPlayer->getFrameSize();
    int width;
    int height;
    std::tie(width, height) = frameSize;
    m_TargetsVector[0]->setHistSize(m_histSize);
    m_particleWidth = m_TargetsVector[0]->getWidth();
    m_particleHeight = m_TargetsVector[0]->getHeight();
    ui->particleWidthLSpinBox->setValue(m_particleWidth);
    ui->particleHeightLSpinBox->setValue(m_particleHeight);
    m_PF = new ParticleFilter(width, height, m_particleCount, m_numIters, m_particleWidth, m_particleHeight, m_histSize,
                              m_TargetsVector[0]);
    ui->particlesToDisplaySlider->setMaximum(m_particleCount);
    m_VideoPlayer->setPF(m_PF);
}

ParticleFilterWidgetGui::~ParticleFilterWidgetGui()
{
    delete m_VideoPlayer;
    delete ui;
}

void ParticleFilterWidgetGui::updatePlayerUI(QImage img)
{
    if (!img.isNull())
    {
        ui->display_label->setAlignment(Qt::AlignCenter);
        ui->display_label->setPixmap(
            QPixmap::fromImage(img).scaled(ui->display_label->size(), Qt::KeepAspectRatio, Qt::FastTransformation)
        );
        ui->horizontalSlider->setValue(m_VideoPlayer->getCurrentFrame());
    }
    int ratio = m_PF->getRatioOfTop(ui->particlesToDisplaySlider->value());
    ui->top_n_ratio_label->setText(QString::number(ratio));
    ui->top_n_ratio_label->repaint();
}

QString ParticleFilterWidgetGui::getFormattedTime(int timeInSeconds)
{
    int seconds = (int)(timeInSeconds) % 60 ;
    int minutes = (int)((timeInSeconds / 60) % 60);
    int hours   = (int)((timeInSeconds / (60 * 60)) % 24);
    QTime t(hours, minutes, seconds);
    if (hours == 0)
        return t.toString("mm:ss");
    else
        return t.toString("h:mm:ss");
}

void ParticleFilterWidgetGui::onHorizontalSliderPressed()
{
    m_isPlaying = !m_VideoPlayer->isStopped();
    m_VideoPlayer->stopVideo();
}

void ParticleFilterWidgetGui::onHorizontalSliderReleased()
{
    m_VideoPlayer->setCurrentFrame(m_currFrame);
    if(m_PF)
        m_PF->reInitialiaze();
    if (m_isPlaying)
        m_VideoPlayer->playVideo();
}

void ParticleFilterWidgetGui::onHorizontalSliderMoved(int position)
{
    m_currFrame = position;
}

void ParticleFilterWidgetGui::onSetParticleCountSliderMoved(int position)
{
    ui->particlesToDisplayLabel->setText(QString::number(position));
    m_PF->setParticlesToDisplay(position);
    //    mPF->showTopNParticles(position);
    int ratio = m_PF->getRatioOfTop(position);
    ui->top_n_ratio_label->setText(QString::number(ratio));
    ui->top_n_ratio_label->repaint();
}

void ParticleFilterWidgetGui::onActionOpen()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open a File", "", "Video File (*.*)");
    QFileInfo name = filename;
    if (!m_VideoPlayer->loadVideo(filename.toStdString()))
    {
        QMessageBox msgBox;
        msgBox.setText(" Selected video could not be opened! ");
        msgBox.exec();
    }
    else
    {
        setWindowTitle(name.fileName());
        ui->start_pushButton->setEnabled(true);
        ui->horizontalSlider->setEnabled(true);
        ui->particlesToDisplaySlider->setEnabled(true);
        ui->horizontalSlider->setMaximum(m_VideoPlayer->getNumberOfFrames());
        std::tuple<int, int> framesize = m_VideoPlayer->getFrameSize();
        int frameWidth;
        int frameHeight;
        std::tie(frameWidth, frameHeight) = framesize;
        m_VideoLodaded = true;
    }
}

void ParticleFilterWidgetGui::onParticleCountChange(int value)
{
    m_particleCount = value;
}

void ParticleFilterWidgetGui::onIterationCountChange(int value)
{
    m_numIters = value;
}

void ParticleFilterWidgetGui::onParticleWidthChange(int value)
{
    m_particleWidth = value;
}

void ParticleFilterWidgetGui::onParticleHeightChange(int value)
{
    m_particleHeight = value;
}

void ParticleFilterWidgetGui::onHistSizeChanged(int value)
{
    m_histSize = value;
}


void ParticleFilterWidgetGui::onActionPlay()
{
    if (m_VideoLodaded)
    {
        m_VideoPlayer->playVideo();
        setPFSettingsEnabled(false);
    }
}

void ParticleFilterWidgetGui::onActionPause()
{
    if (m_VideoLodaded)
    {
        m_VideoPlayer->stopVideo();
        setPFSettingsEnabled(true);
    }
}


void ParticleFilterWidgetGui::setPFSettingsEnabled(bool state)
{
    //    ui->groupBox->setEnabled(state);
    ui->particleCountSpinBox->setEnabled(state);
    ui->iterCountSpinBox->setEnabled(state);
    ui->particleWidthLSpinBox->setEnabled(state);
}
