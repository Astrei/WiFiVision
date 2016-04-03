#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtCore>
#include <QtGui>
#include <QGraphicsScene>
#include <QRectF>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVariant>
#include <QApplication>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QListWidget *listWidget;

public:
    void addItem(QString text, int data);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static const int vm = 20; //Количество строк
    static const int hm = 30; //Количество столбцов
    static const int Kf = 20; //Коэффициент фильтрации ошибок измерения
    static const int Ks = 8; //Коэффициент размера пикселей
    static const int Kr = 30; //Коэффициент количества отображаемых точек доступа
    int pic[vm*Kr][hm]; //Массив значений для построения изображения

private slots:
    void readSerial(); //Функция чтения из порта
    void onItemClicked(QListWidgetItem *item);



    void on_UpButton_clicked();//Нажатие на кнопку "Вверх"
    void on_pushButton_clicked();//Нажатие на "Скан"
    void command(QString);// Строка идентификатиро команды



    void on_rightButton_clicked();

    void on_leftButton_clicked();

    void on_downButton_clicked();



private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsRectItem *pix;

    QSerialPort *arduino;
    static const quint16 arduino_uno_vendor_id = 1027;
    static const quint16 arduino_uno_product_id = 24577;



    QByteArray serialData;
    QString serialBuffer;
    QStringList aplistfin; //Список с только что полученными WiFi точками
    QStringList aplistfin_max; //Список с максимальным количеством найденных точек
};

#endif // MAINWINDOW_H
