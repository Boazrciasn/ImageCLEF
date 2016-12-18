#include "ReaderGUI.h"
#include "ui_ReaderGUI.h"

ReaderGUI::ReaderGUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReaderGUI)
{
    ui->setupUi(this);
    m_reader = new Reader();
    m_DS = new DataSet();
    m_dFlag = ui->DatasetType_comboBox->currentIndex();
    readSettings();
}

ReaderGUI::~ReaderGUI()
{
    delete ui;
}


void ReaderGUI::load()
{
    switch (m_dFlag)
    {
    case Type_Standard:
    {
        m_dirStandard = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"), m_dirStandard,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        m_reader->readImages(m_dirStandard, m_DS->m_ImagesVector,m_dFlag);
        m_DS->m_labels = std::vector<int> (m_DS->m_ImagesVector.size(), 1);

        // FIXME: convert to file traversal
        QString dirNeg = "/home/neko/Desktop/trackingData/INRIAPerson/train_64x128_H96/neg";
        m_reader->readImages(dirNeg, m_DS->m_ImagesVector,m_dFlag);
        m_DS->m_labels.resize(m_DS->m_ImagesVector.size());

        for(auto &img :  m_DS->m_ImagesVector)
            cv::resize(img,img,cv::Size(64,128));

        if (!m_DS->m_ImagesVector.empty())
        {
            ui->LoadedDataset_label->setText(m_dirStandard);
            emit imagesLoaded(true);
        }
        break;
    }
    case Type_MNIST:
    {
        m_dirMNIST = QFileDialog::getOpenFileName(this, tr("Open Dataset File"), m_dirMNIST);
        if (!m_dirMNIST.isEmpty())
        {
            m_reader->readImages(m_dirMNIST, m_DS->m_ImagesVector, m_dFlag);
            if (!m_DS->m_ImagesVector.empty())
            {
                ui->LoadedDataset_label->setText(m_dirMNIST);
                emit imagesLoaded(true);
            }
            m_dirMNIST.replace(QRegExp("images.idx3"), "labels.idx1");
            m_reader->readLabels(m_dirMNIST, m_DS->m_labels, m_dFlag);
            if (!m_DS->m_labels.empty())
            {
                ui->labelFilepath_label->setText(m_dirMNIST);
                emit labelsLoaded(true);
            }
        }
        break;
    }
    default:
        break;
    }
}

void ReaderGUI::dataTypeChanged(int index)
{
    if(index == Type_MNIST)
        ui->loadLabels_checkBox->hide();
    else
        ui->loadLabels_checkBox->show();

    m_dFlag = index;
}

void ReaderGUI::writeSettings()
{
    QSettings settings("VVGLab", "ReaderGUI");
    settings.beginGroup("path");
    settings.setValue("m_dirMNIST", m_dirMNIST);
    settings.setValue("m_dirStandard", m_dirStandard);
    settings.endGroup();
}

void ReaderGUI::readSettings()
{
    QSettings settings("VVGLab", "RDFDialogGUI");
    settings.beginGroup("PARAMS");
    m_dirMNIST = settings.value("m_dirMNIST", "").toString();
    m_dirStandard = settings.value("m_dirStandard", "").toString();
    settings.endGroup();
}
