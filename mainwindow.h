#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void add_prices();

    void ui_update();

private slots:
    void on_cardInput_returnPressed();

    void on_confirmButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::MainWindow *ui;
    QString nick;
    int cents;
    QList<QPushButton*> price_buttons;
};

#endif // MAINWINDOW_H
