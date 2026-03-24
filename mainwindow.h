#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QSettings>

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
    void clear_prices();
    void ui_update();
    void addTransaction();
    void addJsonlTransaction();
    void applyTheme(bool dark);
    void setupTabs();
    void setupMenuBar();
    void refreshTransactionLog();
    void refreshBalanceView();
    void loadThemePreference();
    void saveThemePreference();

private slots:
    void on_cardInput_returnPressed();
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();
    void onTabChanged(int index);
    void toggleTheme();
    void showPriceEditor();

private:
    Ui::MainWindow *ui;
    QString nick;
    int cents;
    QList<QPushButton*> price_buttons;

    // Tabs
    QTabWidget *tabWidget;

    // Transaction log tab
    QTableWidget *logTable;

    // Balance tab
    QTableWidget *balanceTable;
    QLabel *balanceTotalLabel;

    // Theme
    bool darkMode;
    QSettings *settings;

    // Branding
    QLabel *brandLabel;
};

#endif // MAINWINDOW_H
