#ifndef DATAEXTRACTOR_H
#define DATAEXTRACTOR_H

#include "modules/tracking/videoplayer/VideoProcess.h"
#include "Util.h"

template <typename BSubtractor, typename Detector>
class DataExtractor : public VideoProcess
{
  public:
    DataExtractor(BSubtractor &bsubtract, Detector &detector) :
        m_bsubtractor(bsubtract), m_detector(detector)
    {
    }

    void exec(const cv::Mat &inputImg, cv::Mat &outImg)
    {
        setImage(inputImg);
        cv::Mat fg_img;
        m_bsubtractor.execute(m_img, fg_img);


        // TODO: remove this and related to it later
        cv::Mat fg_clone = fg_img.clone();
        cv::Mat grayImg, grayImg_fg;
        cv::threshold(fg_clone , fg_clone , 100, 255, cv::THRESH_BINARY);
        cv::cvtColor(m_img, grayImg, CV_RGB2GRAY);
        grayImg.copyTo(grayImg_fg,fg_clone);

        std::vector<cv::Rect>  bBox_vec;
        bBox_vec = Util::calculateBoundingBoxRect(fg_img, m_BBoxMinSize, m_BBoxMaxSize, m_aspectMax, m_aspectMin);
        quint32 decision;
        quint32 confidence;

        for (size_t i = 0; i < bBox_vec.size() ; ++i)
        {
            cv::Mat roi(inputImg, bBox_vec[i]);
            cv::Mat roi_gray(grayImg_fg, bBox_vec[i]); // TODO: related to previous to be deleted

            m_detector.compute(roi, decision, confidence);
            if (decision == -1.0f)
                bBox_vec.erase(bBox_vec.begin() + i);
            else if (!m_saveDir.isNull())
            {
                QString sampleno = m_saveDir+"/" + QString::number(m_sampleCount).rightJustified(6, '0') + ".png";
                cv::cvtColor(roi,roi, CV_RGB2BGR);
//                cv::imwrite(sampleno.toStdString(), roi); //TODO: uncomment later
                ++m_sampleCount;

                // TODO: remove later
                QString sampleno_gray = m_saveDir+"/" + QString::number(m_sampleCount).rightJustified(6, '0') + "_gray.png";
                cv::imwrite(sampleno_gray.toStdString(), roi_gray);
            }
        }

        Util::drawBoundingBox(m_img, bBox_vec);
        outImg = m_img;
    }

    void setBBoxSizeConstraints(quint16 minSize, quint16 maxSize)
    {
        m_BBoxMaxSize = maxSize;
        m_BBoxMinSize = minSize;
    }

    void setAspectConstraints(double min, double max)
    {
        m_aspectMax = max;
        m_aspectMin = min;
    }

    void setSaveDir(QString saveDir) { m_saveDir = saveDir; }

  private:
    BSubtractor &m_bsubtractor;
    Detector &m_detector;
    quint16 m_BBoxMinSize = 100;
    quint16 m_BBoxMaxSize = 10000;
    quint32 m_sampleCount = 0;
    double m_aspectMax = 5;
    double m_aspectMin = 0.001;
    QString m_saveDir;

};

#endif // DATAEXTRACTOR_H
