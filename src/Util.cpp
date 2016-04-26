#include "precompiled.h"

#include <iostream>

#include "include/Util.h"

SIGNALS S;

SlotDeneme::SlotDeneme()
{
    QObject::connect(&S, SIGNAL(doIt()), this, SLOT(onDoIt()));
}

void deneme()
{
    emit S.doIt();

    cv::Mat_<float> A(3, 3);
    cv::Mat_<int> B(5, 5);
    A.setTo(0);
    B.setTo(10);

    doForAllPixels<float>(A, [](float pixval) {
        return pixval + 1;
    });

    doForAllPixels<int>(B, [](int pixval) {
        return pixval * 5;
    });
}

cv::Mat Util::toCv(const QImage &image , int cv_type)
{
    return cv::Mat(image.height(), image.width(), cv_type, (void *)image.scanLine(0), image.bytesPerLine());
}

QImage Util::toQt(const cv::Mat &src, QImage::Format format)
{
    quint16 width = src.cols;
    quint16 height = src.rows;

    QImage dest = QImage(width, height, format);

    if(src.type() == CV_8UC3)
    {
        for(int i = 0; i < height; i++)
        {
            const quint8 *pSrc = src.ptr<quint8>(i);
            quint8 *pDest = dest.scanLine(i);
            for(int j = 0; j < width; j++)
            {
                *pDest++ = *pSrc++;
                *pDest++ = *pSrc++;
                *pDest++ = *pSrc++;
            }
        }
    }
    else if(src.type() == CV_8UC1)
    {
        for(int i = 0; i < height; i++)
        {
            const quint8 *pSrc = src.ptr<quint8>(i);
            quint8 *pDest = dest.scanLine(i);
            for (int j = 0; j < width; j++)
            {
                quint8 val = *pSrc++;
                *pDest++ = val;
                *pDest++ = val;
                *pDest++ = val;
            }
        }
    }
    return dest;
}

QString Util::cleanNumberAndPunctuation(QString toClean)
{
    QString puncs ="!'^+%&/()=?_-{}][{½$#£><@.,:;|\"§*/";
    QString toReturn="";
    int i=0;
    int j=toClean.length();

    //find head pos to trim
    while (i<j)
    {
        if (!puncs.contains(toClean[i]))
            break;
        i++;
    }

    //find tail pos to trim
    while (j>i)
    {
        j--;
        if (!puncs.contains(toClean[j]))
            break;
    }

    toReturn = toClean.mid(i,j-i+1);

    //FIXME: got to refactor the code here sorry for bold coding.
    i = 0;
    j=toReturn.length();


    //check if the word starts or ends with a number or a whole number
    bool isNumber;
    toReturn.toInt(&isNumber);


    /* FIXME: if there is a number, return "Numbers/number/.. */

/*  if (!isNumber)
    {
        return toReturn.toLower();
    }
*/
    if (toReturn[i].isDigit() || toReturn[j].isDigit() ) {

        toReturn ="Numbers/" +toReturn;
        return toReturn;
    }


    return toReturn.toLower();
}

void Util::plot(const cv::Mat &hist, QWidget *parent)
{
    HistogramDialog *histDialog = new HistogramDialog(parent);
    histDialog->show();
    histDialog->plot(hist);
}

QString Util::fileNameWithoutPath(QString& filePath){
    int posLastDot = filePath.lastIndexOf(".",-1);
    int posLastSlash= filePath.lastIndexOf("/",-1);
    QString fname = filePath.mid(posLastSlash+1,posLastDot-posLastSlash-1);
    filePath = filePath.mid(0,posLastSlash);
    return fname;
}


void Util::convertToOSRAndBlure(QString srcDir, QString outDir, int ksize){
    QString folder;
    QString file;
    outDir += "_ksize_" + QString::number(ksize);
    QDirIterator ittDir(srcDir, QDir::Dirs | QDir::NoDotAndDotDot |QDir::CaseSensitive) ;

    while (ittDir.hasNext()){
        ittDir.next();
        folder = ittDir.fileName();
        QDirIterator ittFile(srcDir + "/"+ folder,QStringList()<<"*.jpg"<<"*.jpeg",QDir::Files);

        QDir dir_save(outDir + "/" + folder);
        if (!dir_save.exists())
        {
            dir_save.mkpath(".");
            if(!dir_save.exists())
                 qDebug() << "ERROR : " << dir_save << " can not be created!" ;
        }

        while(ittFile.hasNext()){
            ittFile.next();
            file = ittFile.fileName();
            cv::Mat img_bw = cv::imread(srcDir.toStdString() + "/" + folder.toStdString() + "/" + file.toStdString(), 0);
            cv::threshold(img_bw, img_bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
            img_bw = 255 - img_bw;
            cv::blur(img_bw,img_bw,cv::Size(ksize,ksize));

            QImage saveQIM = Util::toQt(img_bw, QImage::Format_RGB888);
            saveQIM.save(outDir + "/" + folder + "/" + file);
        }
    }
}

void Util::calcWidthHightStat(QString srcDir){
    float w_avrg, h_avrg;
    int count;

    QString folder;
    QString file;
    QDirIterator ittDir(srcDir, QDir::Dirs | QDir::NoDotAndDotDot |QDir::CaseSensitive) ;

    QFile outLog(srcDir + "/AverageWidthHight.txt");
    outLog.open(QIODevice::WriteOnly);

    while (ittDir.hasNext()){
        ittDir.next();
        folder = ittDir.fileName();
        QDirIterator ittFile(srcDir + "/"+ folder,QStringList()<<"*.jpg"<<"*.jpeg",QDir::Files);

        w_avrg = h_avrg = 0;
        count = 0;

        while(ittFile.hasNext()){
            ittFile.next();
            file = ittFile.fileName();
            cv::Mat img_bw = cv::imread(srcDir.toStdString() + "/" + folder.toStdString() + "/" + file.toStdString(), 0);

            w_avrg += img_bw.cols;
            h_avrg += img_bw.rows;
            ++count;
            img_bw.release();
        }

        w_avrg /= count;
        h_avrg /= count;

        outLog.write(folder.toStdString().c_str());
        outLog.write(" ");
        outLog.write(QByteArray::number((int)(w_avrg + 0.5f)));
        outLog.write(" ");
        outLog.write(QByteArray::number((int)(h_avrg + 0.5f)));
        outLog.write("\n");

//        QFile textFile(srcDir + "/" + folder + "/" + folder + ".txt");
//        textFile.open(QIODevice::WriteOnly);
//        textFile.write(QByteArray::number(w_avrg));
//        textFile.write(" ");
//        textFile.write(QByteArray::number(h_avrg));
//        textFile.close();
    }
    outLog.close();
}


























