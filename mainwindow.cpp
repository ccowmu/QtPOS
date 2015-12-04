#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <cstdlib>
#include <openssl/md5.h>
#include <ctime>

static QString NICK_DIR = "nicks/",
               ID_DIR = "ids/";

static QString GREEN_STYLESHEET = "color: green;",
               RED_STYLESHEET = "color: red;";

// Number of columns in the grid of prices
static int COLUMNS = 4;

QString md5(QString s) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)s.data(), s.length(), (unsigned char *)&digest);
    QByteArray array((const char *)&digest, MD5_DIGEST_LENGTH);
    return QString(array.toHex());
}

/*
 * Format a (possibly negative) cents value nicely as a dollar amount, including the leading $
 */
QString dollars(int cents) {
    QString prefix("$");
    if (cents < 0) {
        prefix += "-";
    }
    return prefix + QString("%1.%2").arg(abs(cents/100)).arg(abs(cents%100),2,10,QChar('0'));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    nick(QString()),
    cents(0)
{
    ui->setupUi(this);
    add_prices();
    ui->confirmButton->setStyleSheet(GREEN_STYLESHEET);
    ui->cancelButton->setStyleSheet(RED_STYLESHEET);
    ui->cardInput->setEchoMode(QLineEdit::Password);
    ui_update();
    // Ensure the directories we use later are present.
    QDir dir;
    dir.mkdir(ID_DIR);
    dir.mkdir(NICK_DIR);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Parse out the products and prices from "prices.csv"
 *
 * We just assume there are no embedded commas in the "CSV"
 *
 * TODO: We also do not allow any embedded spaces in the fields currently.
 */
void MainWindow::add_prices() {
    int cost, i = 0;
    QString line;
    QStringList splits;
    QFile prices_file("prices.csv");
    if (!prices_file.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("Could not open prices.csv");
    QTextStream prices_stream(&prices_file);
    while (!prices_stream.atEnd()) {
        //TODO: handle spaces
        prices_stream >> line;
        splits = line.split(',');
        if (splits.length() != 2) {
            //TODO: handle other bad line stuff
            break;
        }
        cost = splits[1].toInt();
        // Whole lot of fiddling around to make the buttons layout well and be pretty.
        QPushButton *button = new QPushButton(QString("%1 (%2)").arg(splits[0]).arg(dollars(cost)), this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setStyleSheet(cost >= 0 ? GREEN_STYLESHEET : RED_STYLESHEET);
        button->update();
        // This is important: we set the slot of the button to modify the user's balance by the items cost.
        // So negative costs deduct from the balance and positive costs add to it (perhaps this is backwards...)
        connect(button, &QPushButton::clicked,
                [=]() { cents += cost; ui_update(); });
        price_buttons.push_back(button);
        ui->pricesGrid->addWidget(button, i / COLUMNS, i % COLUMNS);
        i++;
    }
    prices_file.close();
}

/*
 * Enable/disable/update UI elements based on whether someone is "logged in" (nick is not empty)
 */
void MainWindow::ui_update() {
    bool b = !nick.isEmpty();
    for (auto i = price_buttons.begin(); i != price_buttons.end(); ++i) {
        (*i)->setEnabled(b);
    }
    ui->cancelButton->setEnabled(b);
    ui->confirmButton->setEnabled(b);
    ui->centsLabel->setText(dollars(cents));
    ui->centsLabel->setStyleSheet(cents >= 0 ? GREEN_STYLESHEET : RED_STYLESHEET);
    ui->centsLabel->setVisible(b);
    ui->nickLabel->setText(nick);
    ui->nickLabel->setVisible(b);
    ui->cardInput->setFocus();
    ui->cardInput->setVisible(!b);
}

/*
 * When someone has entered an id (or swiped a card).
 */
void MainWindow::on_cardInput_returnPressed()
{
    // Ignore if someone is already "logged in"
    if (!nick.isEmpty()) {
        ui->cardInput->clear();
        return;
    }
    // Otherwise we look to see if the id is already known:
    QString id = md5(ui->cardInput->text());
    ui->cardInput->clear();
    QFile id_file(ID_DIR + id);
    if (!id_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // If we have never seen this id before we prompt for the nickname to associate with it.
        // The id file is named with the md5 sum of the id and contains the nick in plain text
        bool ok;
        do {
             nick = QInputDialog::getText(this, "nick", "nick:", QLineEdit::Normal, "", &ok).remove('/').remove('.');
        } while (!ok || nick.isEmpty());
        QFile id_file(ID_DIR + id);
        if (!id_file.open(QIODevice::WriteOnly | QIODevice::Text))
            qFatal("Could not create id file");
        QTextStream out(&id_file);
        out << nick;
        id_file.close();
    } else {
        // If we have seen this id before just read off the nick associated with it
        QTextStream in(&id_file);
        in >> nick;
        id_file.close();
    }

    // At this point we have the nick of the user, so open the nick file to get the cents the user has
    QFile nick_file(NICK_DIR + nick);
    if (!nick_file.open(QIODevice::ReadWrite | QIODevice::Text))
        qFatal("Could not open nick file for reading");
    QTextStream f(&nick_file);
    f >> cents;
    nick_file.close();

    ui_update();
}

/*
 * When someone has confirmed an order.
 */
void MainWindow::on_confirmButton_clicked()
{
    // Open the nick file and write the balance out to it.
    QFile file(NICK_DIR + nick);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        qFatal("Could not open nick file for writing");
    QTextStream f(&file);
    f << cents;
    file.close();
    addTransaction();
    // "Log out" the user.
    addTransaction();
    nick.clear();
    cents = 0;
    ui_update();
}

/*
 * When someone has cancelled an order.
 */
void MainWindow::on_cancelButton_clicked()
{
    // "Log out" the user.
    nick.clear();
    cents = 0;
    ui_update();
}

void MainWindow::addTransaction()
{
    QFile file("transactions.txt");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        qFatal("Could not open transaction file for writing");

    QTextStream f(&file);
    time_t t = time(0);
    f << t << " "  << nick << " " << cents << "\n";
    file.close();
}



