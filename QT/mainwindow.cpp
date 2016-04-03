#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort>
#include <string>
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QMessageBox>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    listWidget = new QListWidget();

    //Связыаем сигнал клика по списку точек с обработчиком
    QObject::connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(onItemClicked(QListWidgetItem *)));

    //настройка сцены изображения
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);


    arduino = new QSerialPort(this);
    serialBuffer = "";

    /*
     testing code for search arduino's Vendor ID and Product ID
    qDebug()<<"Number of ports:"<<QSerialPortInfo::availablePorts().length()<<"\n";
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
       qDebug()<< "Description:" << serialPortInfo.description()<<"\n";
       qDebug()<< "Has vendor id?" << serialPortInfo.hasVendorIdentifier() <<"\n";
       qDebug()<< "Vendor ID" << serialPortInfo.vendorIdentifier() << "\n";
       qDebug()<< "Has Product ID?" << serialPortInfo.hasProductIdentifier() <<"\n";
       qDebug()<< "Product ID" << serialPortInfo.productIdentifier() <<"\n";
    }*/

    bool arduino_is_available = false;
    QString arduino_uno_port_name;

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasProductIdentifier() &&serialPortInfo.hasVendorIdentifier()){
            if((serialPortInfo.productIdentifier()==arduino_uno_product_id)
                    && (serialPortInfo.vendorIdentifier() ==arduino_uno_vendor_id)){
                arduino_is_available = true;
                 qDebug()<< "Arduino port is available \n";
                arduino_uno_port_name = serialPortInfo.portName();
            }
        }
    }

    if(arduino_is_available){
        qDebug()<< "Found the arduino port...\n";
        arduino->setPortName(arduino_uno_port_name);
        arduino->open(QSerialPort::ReadWrite);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setDataTerminalReady(true);
        QObject::connect(arduino, SIGNAL(readyRead()),this, SLOT(readSerial()));
    } else{
        qDebug()<<"Could not find the correct port for the arduino. \n";
        QMessageBox::information(this,"Serial port ERROR","Could not open serial port for Arduino.");
    }
}

MainWindow::~MainWindow()
{
    if(arduino->isOpen()){
        arduino->close();
    }

    delete ui;
}

void MainWindow::readSerial()
{
    QByteArray data = arduino->readAll();
    serialBuffer += QString::fromStdString(data.toStdString());
}

void MainWindow::command(QString command){ //Посылка команды по Serial
    if(arduino->isWritable()){
arduino->write(command.toStdString().c_str());
    }else{
        qDebug() << "Could not write to Serial!";
    }
}

void MainWindow::on_UpButton_clicked()
{
    MainWindow::command(QString("u"));
}

void MainWindow::on_downButton_clicked()
{
    MainWindow::command(QString("d"));
}


void MainWindow::on_rightButton_clicked()
{
    MainWindow::command(QString("r"));
}

void MainWindow::on_leftButton_clicked()
{
    MainWindow::command(QString("l"));
}


void MainWindow::on_pushButton_clicked()
{

    //Обновляем интерфейс
   scene->clear();
   ui->label->setText("Scanning...");
   ui->UpButton->setEnabled(false);
   ui->downButton->setEnabled(false);
   ui->rightButton->setEnabled(false);
   ui->leftButton->setEnabled(false);
   ui->pushButton->setEnabled(false);


   int RSSI = 0; //Переменная уровня сигнала (дБ)
   int max_signal = -255;//Максимальный сигнал в массиве
   int min_signal = 0;//Минимальный сигнал в массиве
   int v = 0;
   int h = 0;

   for(v=0;v<vm;v=v+1){

        MainWindow::command(QString("d")); //вертикальное движение
        arduino->waitForReadyRead(200);
   for(h=0;h<hm;h=h+1){
 if (v&1) {MainWindow::command(QString("l"));} //горизонтальное движение
   else{MainWindow::command("r");}
        arduino->waitForReadyRead(200);
        MainWindow::command(QString("t"));
        arduino->waitForReadyRead(4000);//ждем пока Arduino пришлет данные
    //Вылавливаем нужные данные из полученного по Serial

        QStringList alllist = serialBuffer.split("\r\n");
        QStringList aplist = alllist.filter ("CWLAP:");

        for (int i=0; i<aplist.size();i++){
         QString apstr = aplist.at(i);
         QString name = apstr.section(',',1,1);
         QString name2 = name.section(QRegExp("\""),1,1);

        //Если массив содержит найденную точку доступа, то записываем сигнал с нее
    if (aplistfin_max.contains(name2)){
          QString RSSI_str = apstr.section(',',2,2);
          bool ok;
          RSSI = RSSI_str.toInt(&ok,10);
          int index = (aplistfin_max.indexOf(name2));
          if (index<=10){
          pic[vm*index+v][h] = RSSI;
          }

        //Если нет, то добавляем ее в список
            }else{
            aplistfin_max += name2;
            QString RSSI_str = apstr.section(',',2,2);
            bool ok;
            RSSI = RSSI_str.toInt(&ok,10);
            int index = aplistfin_max.indexOf(name2);
            if (index<=Kr){
            pic[vm*index+v][h] = RSSI;}
            }

        //Выводим на экран отсканированные точки доступа
            ui->listWidget->clear();
           for (int i=0; i<aplistfin_max.size();i++){
             addItem(aplistfin_max.at(i),i);}
            }
        //Записываем значение сигнала в двумерный массив и вычисляем максимальный и минимальный уровень
        serialBuffer.clear();

        QRgb color = qRgb(255 + 2*RSSI,0,0);//цвет пикселя
        QBrush Brush(color);//кисть
        QPen pen(color);//Цвет границы (она не нужна)
        pen.setWidth(0);//Толщина границы - ноль

   if(v&1) {pix = scene->addRect((hm-1)*Ks-h*Ks,-Ks+v*Ks,Ks,Ks,pen,Brush);}
   else {pix = scene->addRect(h*Ks,-Ks+v*Ks,Ks,Ks,pen,Brush);}//Рисуем пиксель
   qApp->processEvents();


   //Сканирование завершено
   if (((v+1) == vm)&&((h+1) == hm)){

            //Обновление интерфейса
            scene->clear();
            ui->label->setText("Scan ready");
            ui->UpButton->setEnabled(true);
            ui->downButton->setEnabled(true);
            ui->rightButton->setEnabled(true);
            ui->leftButton->setEnabled(true);
            ui->pushButton->setEnabled(true);
            ui->listWidget->item(0)->setSelected(true);


        //Введем небольшую фильтрацию сильно выходящих за пределы измерения значений
        //Находим минимальное и максимальное значения
            for(int va=0;va<vm;va=va+1){
                for(int ha=0;ha<hm;ha=ha+1){
            if(max_signal<pic[va][ha])
            {max_signal = pic[va][ha];}
            if(min_signal>pic[va][ha])
            {min_signal = pic[va][ha];};
                }}

            int summ = 0;
            for (int g = 0; g < vm; g++){
                for (int l = 0; l< hm; l++){
                    summ += pic[g][l];
               }
            }
        //Находим среднее арифметическое
        int average_sign = summ / (vm*hm);
        //Убираем грубые ошибки измерения
        for(int vb=0;vb<vm;vb=vb+1){
            for(int hb=0;hb<hm;hb=hb+1){
        if(max_signal>average_sign + Kf)
            {max_signal = average_sign;
            pic[vb][hb] = max_signal;}
        if(min_signal<average_sign - Kf)
            {min_signal = average_sign;
             pic[vb][hb] = min_signal;};
            }}

        //Нулевые значения автоматически присваивают минимальный сигнал
        for(int vc=0;vc<vm;vc=vc+1){
            for(int hc=0;hc<hm;hc=hc+1){
        if(pic[vc][hc] == 0)
        {pic[vc][hc] = min_signal;}}}

        //Считаем разницу максимального и минимального сигналов для расчета цвета
        int difference_sign = max_signal - min_signal;
        if (difference_sign == 0){difference_sign=1;}
        int color_correction = 255/difference_sign;
            for(int ve=0;ve<vm;ve=ve+1){
                for(int he=0;he<hm;he=he+1){

   int RGB_color = (pic[ve][he] - min_signal)*color_correction;
   QRgb color = qRgb(0,0,RGB_color);//цвет пикселя
   QBrush Brush(color);//кисть
   QPen pen(color);//Цвет границы (она не нужна)
   pen.setWidth(0);//Толщина границы - ноль


   if(ve&1) {pix = scene->addRect((hm-1)*Ks-he*Ks,-Ks+ve*Ks,Ks,Ks,pen,Brush); }
   else {pix = scene->addRect(he*Ks,-Ks+ve*Ks,Ks,Ks,pen,Brush);}//Рисуем пиксель
}}

   }
}
}}




void MainWindow::addItem(QString text, int data)
{
    QListWidgetItem *item = new QListWidgetItem(text);
    QVariant variant(data);
    item->setData(Qt::UserRole, variant);
    ui->listWidget->addItem(item);
}

//Обработчик выбора точки из списка для отрисовки новой картинки
void MainWindow::onItemClicked(QListWidgetItem *item)
{
    QVariant variant = item->data(Qt::UserRole);
    int data = (variant.toInt());

            //Обновление интерфейса
                scene->clear();
                int max_signal = -255;//Максимальный сигнал в массиве
                int min_signal = 0;//Минимальный сигнал в массиве

            //Введем небольшую фильтрацию сильно выходящих за пределы измерения значений
            //Находим минимальное и максимальное значения
                for(int va=0;((vm*data)+va)<(vm*(data+1)); va=va+1){
                    for(int ha=0;ha<hm;ha=ha+1){
                if(max_signal<pic[(vm*data)+va][ha])
                {max_signal = pic[(vm*data)+va][ha];}
                if(min_signal>pic[(vm*data)+va][ha])
                {min_signal = pic[(vm*data)+va][ha];};
                    }}

                int summ = 0;
                for (int g = 0;((vm*data)+g)<(vm*(data+1)); g++){
                    for (int l = 0; l< hm; l++){
                        summ += pic[(vm*data)+g][l];
                   }
                }
            //Находим среднее арифметическое
            int average_sign = summ / (vm*hm);

            //Убираем грубые ошибки измерения
            for(int vb=0;((vm*data)+vb)<(vm*(data+1));vb=vb+1){
                for(int hb=0;hb<hm;hb=hb+1){
            if(max_signal>average_sign + Kf)
                {max_signal = average_sign;
                pic[(vm*data)+vb][hb] = max_signal;}
            if(min_signal<average_sign - Kf)
                {min_signal = average_sign;
                 pic[(vm*data)+vb][hb] = min_signal;};
                }}

            //Нулевые значения автоматически присваивают минимальный сигнал
            for(int vc=0;((vm*data)+vc)<(vm*(data+1));vc=vc+1){
                for(int hc=0;hc<hm;hc=hc+1){
            if(pic[(vm*data)+vc][hc] == 0)
            {pic[(vm*data)+vc][hc] = min_signal;}}}

            //Считаем разницу максимального и минимального сигналов для расчета цвета
            int difference_sign = max_signal - min_signal;
            if (difference_sign == 0){difference_sign=1;}
            int color_correction = 255/difference_sign;
                for(int ve=0;((vm*data)+ve)<(vm*(data+1));ve=ve+1){
                    for(int he=0;he<hm;he=he+1){
       int RGB_color = (pic[(vm*data)+ve][he] - min_signal)*color_correction;
       QRgb color = qRgb(0,0,RGB_color);//цвет пикселя
       QBrush Brush(color);//кисть
       QPen pen(color);//Цвет границы (она не нужна)
       pen.setWidth(0);//Толщина границы - ноль

       if(ve&1) {pix = scene->addRect((hm-1)*Ks-he*Ks,-Ks+ve*Ks,Ks,Ks,pen,Brush); }
       else {pix = scene->addRect(he*Ks,-Ks+ve*Ks,Ks,Ks,pen,Brush);}//Рисуем пиксель
    }

       }

}
