#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QJsonObject>
#include <QJsonDocument>
#include <cstdlib>
#include <openssl/md5.h>

static QString NICK_DIR = "nicks/",
               ID_DIR = "ids/";

static int COLUMNS = 4;

// --- Branding colors from cclub.cs.wmich.edu ---
static const char* DARK_STYLE = R"(
    QMainWindow, QWidget { background-color: #0a0a0a; color: #e0e0e0; }
    QTabWidget::pane { border: 1px solid #404040; background: #1a1a1a; }
    QTabBar::tab { background: #1a1a1a; color: #b0b0b0; padding: 8px 16px;
                   border: 1px solid #404040; border-bottom: none; margin-right: 2px; }
    QTabBar::tab:selected { background: #0a0a0a; color: #4a9eff; border-bottom: 2px solid #4a9eff; }
    QTabBar::tab:hover { color: #e0e0e0; }
    QPushButton { background-color: #1a1a1a; color: #e0e0e0; border: 1px solid #404040;
                  padding: 8px; font-size: 14px; border-radius: 4px; }
    QPushButton:hover { background-color: #2a2a2a; }
    QPushButton:disabled { color: #555555; background-color: #111111; }
    QPushButton#confirmButton { background-color: #1a3a1a; color: #4eff4a; border-color: #2a5a2a; }
    QPushButton#confirmButton:hover { background-color: #2a4a2a; }
    QPushButton#cancelButton { background-color: #3a1a1a; color: #ff4a4a; border-color: #5a2a2a; }
    QPushButton#cancelButton:hover { background-color: #4a2a2a; }
    QLineEdit { background-color: #1a1a1a; color: #e0e0e0; border: 1px solid #404040;
                padding: 6px; border-radius: 4px; }
    QLabel { color: #e0e0e0; }
    QLabel#brandLabel { color: #4a9eff; font-size: 18px; font-weight: bold; }
    QTableWidget { background-color: #1a1a1a; color: #e0e0e0; border: 1px solid #404040;
                   gridline-color: #333333; }
    QTableWidget::item { padding: 4px; }
    QTableWidget::item:selected { background-color: #2a3a5a; }
    QHeaderView::section { background-color: #0a0a0a; color: #4a9eff; border: 1px solid #404040;
                           padding: 4px; font-weight: bold; }
    QMenuBar { background-color: #0a0a0a; color: #e0e0e0; border-bottom: 1px solid #404040; }
    QMenuBar::item:selected { background-color: #2a2a2a; }
    QMenu { background-color: #1a1a1a; color: #e0e0e0; border: 1px solid #404040; }
    QMenu::item:selected { background-color: #2a3a5a; }
    QStatusBar { background-color: #0a0a0a; color: #b0b0b0; }
)";

static const char* LIGHT_STYLE = R"(
    QMainWindow, QWidget { background-color: #f5f7fa; color: #212529; }
    QTabWidget::pane { border: 1px solid #dee2e6; background: #ffffff; }
    QTabBar::tab { background: #e9ecef; color: #495057; padding: 8px 16px;
                   border: 1px solid #dee2e6; border-bottom: none; margin-right: 2px; }
    QTabBar::tab:selected { background: #ffffff; color: #4a9eff; border-bottom: 2px solid #4a9eff; }
    QTabBar::tab:hover { color: #212529; }
    QPushButton { background-color: #e9ecef; color: #212529; border: 1px solid #dee2e6;
                  padding: 8px; font-size: 14px; border-radius: 4px; }
    QPushButton:hover { background-color: #d0d4d9; }
    QPushButton:disabled { color: #adb5bd; background-color: #f0f0f0; }
    QPushButton#confirmButton { background-color: #d4edda; color: #155724; border-color: #c3e6cb; }
    QPushButton#confirmButton:hover { background-color: #c3e6cb; }
    QPushButton#cancelButton { background-color: #f8d7da; color: #721c24; border-color: #f5c6cb; }
    QPushButton#cancelButton:hover { background-color: #f5c6cb; }
    QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #dee2e6;
                padding: 6px; border-radius: 4px; }
    QLabel { color: #212529; }
    QLabel#brandLabel { color: #4a9eff; font-size: 18px; font-weight: bold; }
    QTableWidget { background-color: #ffffff; color: #212529; border: 1px solid #dee2e6;
                   gridline-color: #dee2e6; }
    QTableWidget::item { padding: 4px; }
    QTableWidget::item:selected { background-color: #cce5ff; }
    QHeaderView::section { background-color: #f5f7fa; color: #4a9eff; border: 1px solid #dee2e6;
                           padding: 4px; font-weight: bold; }
    QMenuBar { background-color: #f5f7fa; color: #212529; border-bottom: 1px solid #dee2e6; }
    QMenuBar::item:selected { background-color: #e9ecef; }
    QMenu { background-color: #ffffff; color: #212529; border: 1px solid #dee2e6; }
    QMenu::item:selected { background-color: #cce5ff; }
    QStatusBar { background-color: #f5f7fa; color: #495057; }
)";

QString md5(QString s) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)s.data(), s.length(), (unsigned char *)&digest);
    QByteArray array((const char *)&digest, MD5_DIGEST_LENGTH);
    return QString(array.toHex());
}

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
    cents(0),
    darkMode(true)
{
    ui->setupUi(this);

    settings = new QSettings("CCaWMU", "POS", this);

    QDir dir;
    dir.mkdir(ID_DIR);
    dir.mkdir(NICK_DIR);

    ui->confirmButton->setObjectName("confirmButton");
    ui->cancelButton->setObjectName("cancelButton");
    ui->cardInput->setEchoMode(QLineEdit::Password);

    setupMenuBar();
    setupTabs();
    add_prices();
    loadThemePreference();
    applyTheme(darkMode);
    ui_update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    QMenu *adminMenu = menuBar->addMenu("Admin");

    QAction *priceAction = new QAction("Edit Prices", this);
    connect(priceAction, &QAction::triggered, this, &MainWindow::showPriceEditor);
    adminMenu->addAction(priceAction);

    adminMenu->addSeparator();

    QAction *themeAction = new QAction("Toggle Dark/Light Mode", this);
    connect(themeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
    adminMenu->addAction(themeAction);
}

void MainWindow::setupTabs()
{
    tabWidget = new QTabWidget(this);

    QWidget *posTab = ui->centralWidget;
    QVBoxLayout *posLayout = qobject_cast<QVBoxLayout*>(
        posTab->layout()->itemAt(0)->layout());
    if (posLayout) {
        brandLabel = new QLabel("CCaWMU POS", posTab);
        brandLabel->setObjectName("brandLabel");
        brandLabel->setAlignment(Qt::AlignCenter);
        posLayout->insertWidget(0, brandLabel);
    }
    tabWidget->addTab(posTab, "POS");

    QWidget *logTab = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(logTab);
    logTable = new QTableWidget(logTab);
    logTable->setColumnCount(3);
    logTable->setHorizontalHeaderLabels({"Timestamp", "User", "Balance After"});
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setSortingEnabled(true);
    logLayout->addWidget(logTable);
    tabWidget->addTab(logTab, "Transaction Log");

    QWidget *balanceTab = new QWidget();
    QVBoxLayout *balLayout = new QVBoxLayout(balanceTab);
    balanceTable = new QTableWidget(balanceTab);
    balanceTable->setColumnCount(3);
    balanceTable->setHorizontalHeaderLabels({"User", "Cents", "USD"});
    balanceTable->horizontalHeader()->setStretchLastSection(true);
    balanceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    balanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    balanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    balanceTable->setSortingEnabled(true);
    balLayout->addWidget(balanceTable);
    balanceTotalLabel = new QLabel("Grand Total: $0.00", balanceTab);
    balanceTotalLabel->setAlignment(Qt::AlignRight);
    balanceTotalLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 8px;");
    balLayout->addWidget(balanceTotalLabel);
    tabWidget->addTab(balanceTab, "Balances");

    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    setCentralWidget(tabWidget);
}

void MainWindow::onTabChanged(int index)
{
    if (index == 1) refreshTransactionLog();
    else if (index == 2) refreshBalanceView();
}

void MainWindow::refreshTransactionLog()
{
    QFile file("transactions.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logTable->setRowCount(0);
        return;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) lines.append(line);
    }
    file.close();

    int start = qMax(0, lines.size() - 200);
    int count = lines.size() - start;

    logTable->setSortingEnabled(false);
    logTable->setRowCount(count);

    for (int i = 0; i < count; i++) {
        QStringList parts = lines[start + i].split(' ');
        if (parts.size() < 3) continue;

        QString timestamp = parts[0];
        QString balance = parts.last();
        QStringList nickParts = parts.mid(1, parts.size() - 2);
        QString user = nickParts.join(' ');

        int row = count - 1 - i;
        logTable->setItem(row, 0, new QTableWidgetItem(timestamp));
        logTable->setItem(row, 1, new QTableWidgetItem(user));

        QTableWidgetItem *balItem = new QTableWidgetItem();
        bool ok;
        int balVal = balance.toInt(&ok);
        if (ok) {
            balItem->setData(Qt::DisplayRole, balVal);
            balItem->setText(dollars(balVal));
            balItem->setForeground(balVal >= 0 ? QColor("#4eff4a") : QColor("#ff4a4a"));
        } else {
            balItem->setText(balance);
        }
        logTable->setItem(row, 2, balItem);
    }
    logTable->setSortingEnabled(true);
}

void MainWindow::refreshBalanceView()
{
    QDir nicksDir(NICK_DIR);
    if (!nicksDir.exists()) { balanceTable->setRowCount(0); return; }

    QStringList files = nicksDir.entryList(QDir::Files);
    struct UserBal { QString name; int balance; };
    QList<UserBal> users;

    for (const QString &name : files) {
        QFile f(NICK_DIR + name);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream in(&f);
        int val = 0;
        in >> val;
        f.close();
        if (val != 0) users.append({name, val});
    }

    std::sort(users.begin(), users.end(),
              [](const UserBal &a, const UserBal &b) { return a.balance < b.balance; });

    balanceTable->setSortingEnabled(false);
    balanceTable->setRowCount(users.size());

    int grandTotal = 0;
    for (int i = 0; i < users.size(); i++) {
        const UserBal &u = users[i];
        grandTotal += u.balance;
        balanceTable->setItem(i, 0, new QTableWidgetItem(u.name));
        QTableWidgetItem *centsItem = new QTableWidgetItem();
        centsItem->setData(Qt::DisplayRole, u.balance);
        centsItem->setForeground(u.balance >= 0 ? QColor("#4eff4a") : QColor("#ff4a4a"));
        balanceTable->setItem(i, 1, centsItem);
        QTableWidgetItem *usdItem = new QTableWidgetItem(dollars(u.balance));
        usdItem->setForeground(u.balance >= 0 ? QColor("#4eff4a") : QColor("#ff4a4a"));
        balanceTable->setItem(i, 2, usdItem);
    }
    balanceTable->setSortingEnabled(true);

    balanceTotalLabel->setText(QString("Grand Total: %1  |  %2 users").arg(dollars(grandTotal)).arg(users.size()));
    balanceTotalLabel->setStyleSheet(
        QString("font-size: 16px; font-weight: bold; padding: 8px; color: %1;")
            .arg(grandTotal >= 0 ? "#4eff4a" : "#ff4a4a"));
}

void MainWindow::add_prices() {
    int cost, i = 0;
    QFile prices_file("prices.csv");
    if (!prices_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Warning: Could not open prices.csv", 5000);
        return;
    }
    QTextStream prices_stream(&prices_file);
    while (!prices_stream.atEnd()) {
        QString line = prices_stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList splits = line.split(',');
        if (splits.length() != 2) { qDebug() << "Skipping malformed:" << line; continue; }
        bool ok;
        cost = splits[1].trimmed().toInt(&ok);
        if (!ok) { qDebug() << "Skipping non-numeric:" << line; continue; }

        QPushButton *button = new QPushButton(
            QString("%1 (%2)").arg(splits[0].trimmed()).arg(dollars(cost)), this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(button, &QPushButton::clicked, [=]() { cents += cost; ui_update(); });
        price_buttons.push_back(button);
        ui->pricesGrid->addWidget(button, i / COLUMNS, i % COLUMNS);
        i++;
    }
    prices_file.close();
}

void MainWindow::clear_prices() {
    for (QPushButton *btn : price_buttons) {
        ui->pricesGrid->removeWidget(btn);
        delete btn;
    }
    price_buttons.clear();
}

void MainWindow::ui_update() {
    bool b = !nick.isEmpty();
    for (auto it = price_buttons.begin(); it != price_buttons.end(); ++it) (*it)->setEnabled(b);
    ui->cancelButton->setEnabled(b);
    ui->confirmButton->setEnabled(b);
    ui->centsLabel->setText(dollars(cents));
    ui->centsLabel->setStyleSheet(QString("color: %1; font-size: 26px;").arg(cents >= 0 ? "#4eff4a" : "#ff4a4a"));
    ui->centsLabel->setVisible(b);
    ui->nickLabel->setText(nick);
    ui->nickLabel->setVisible(b);
    ui->cardInput->setFocus();
    ui->cardInput->setVisible(!b);
}

void MainWindow::on_cardInput_returnPressed()
{
    if (!nick.isEmpty()) { ui->cardInput->clear(); return; }
    QString id_in = ui->cardInput->text();
    if (id_in.length() != 38) { ui->cardInput->clear(); return; }
    QString id = md5(ui->cardInput->text());
    ui->cardInput->clear();

    QFile id_file(ID_DIR + id);
    if (!id_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        bool ok;
        nick = QInputDialog::getText(this, "nick", "nick:", QLineEdit::Normal, "", &ok)
                   .remove('/').remove('.').trimmed().toLower();
        if (!ok || nick.isEmpty()) return;
        QFile new_id_file(ID_DIR + id);
        if (!new_id_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            statusBar()->showMessage("Error: Could not create id file", 5000);
            nick.clear(); return;
        }
        QTextStream out(&new_id_file);
        out << nick;
        new_id_file.close();
    } else {
        QTextStream in(&id_file);
        in >> nick;
        id_file.close();
        nick = nick.toLower();
    }

    QFile nick_file(NICK_DIR + nick);
    if (!nick_file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        if (!nick_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            statusBar()->showMessage("Error: Could not open nick file", 5000);
            nick.clear(); return;
        }
        QTextStream f(&nick_file);
        f << 0;
        nick_file.close();
        cents = 0;
    } else {
        QTextStream f(&nick_file);
        f >> cents;
        nick_file.close();
    }
    ui_update();
}

void MainWindow::on_confirmButton_clicked()
{
    QFile file(NICK_DIR + nick);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        statusBar()->showMessage("Error: Could not save balance", 5000);
        return;
    }
    QTextStream f(&file);
    f << cents;
    file.close();
    addTransaction();
    addJsonlTransaction();
    statusBar()->showMessage(QString("%1: %2").arg(nick).arg(dollars(cents)), 3000);
    nick.clear();
    cents = 0;
    ui_update();
}

void MainWindow::on_cancelButton_clicked()
{
    nick.clear();
    cents = 0;
    ui_update();
}

void MainWindow::addTransaction()
{
    QFile file("transactions.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        statusBar()->showMessage("Warning: Could not log transaction", 5000);
        return;
    }
    QTextStream f(&file);
    f << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << nick << " " << cents << "\n";
    file.close();
}

void MainWindow::addJsonlTransaction()
{
    QFile file("transactions.jsonl");
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;
    QJsonObject obj;
    obj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    obj["user"] = nick;
    obj["balance_after"] = cents;
    QTextStream f(&file);
    f << QJsonDocument(obj).toJson(QJsonDocument::Compact) << "\n";
    file.close();
}

void MainWindow::applyTheme(bool dark) { darkMode = dark; setStyleSheet(dark ? DARK_STYLE : LIGHT_STYLE); saveThemePreference(); }
void MainWindow::toggleTheme() { applyTheme(!darkMode); }
void MainWindow::loadThemePreference() { darkMode = settings->value("theme/dark", true).toBool(); }
void MainWindow::saveThemePreference() { settings->setValue("theme/dark", darkMode); }

void MainWindow::showPriceEditor()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Prices");
    dialog.resize(400, 350);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTableWidget *table = new QTableWidget(&dialog);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Item Name", "Price (cents)"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    QFile prices_file("prices.csv");
    if (prices_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&prices_file);
        int row = 0;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            QStringList parts = line.split(',');
            if (parts.size() != 2) continue;
            table->setRowCount(row + 1);
            table->setItem(row, 0, new QTableWidgetItem(parts[0].trimmed()));
            QTableWidgetItem *priceItem = new QTableWidgetItem();
            bool ok; int val = parts[1].trimmed().toInt(&ok);
            if (ok) priceItem->setData(Qt::EditRole, val);
            else priceItem->setText(parts[1].trimmed());
            table->setItem(row, 1, priceItem);
            row++;
        }
        prices_file.close();
    }
    layout->addWidget(table);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("Add Row", &dialog);
    QPushButton *removeBtn = new QPushButton("Remove Selected", &dialog);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    layout->addLayout(btnLayout);

    connect(addBtn, &QPushButton::clicked, [&]() {
        int row = table->rowCount();
        table->setRowCount(row + 1);
        table->setItem(row, 0, new QTableWidgetItem("New Item"));
        QTableWidgetItem *p = new QTableWidgetItem();
        p->setData(Qt::EditRole, -100);
        table->setItem(row, 1, p);
    });
    connect(removeBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        if (row >= 0) table->removeRow(row);
    });

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    if (dialog.exec() != QDialog::Accepted) return;

    for (int i = 0; i < table->rowCount(); i++) {
        QTableWidgetItem *nameItem = table->item(i, 0);
        QTableWidgetItem *priceItem = table->item(i, 1);
        if (!nameItem || nameItem->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Error", QString("Row %1: empty name.").arg(i+1)); return;
        }
        if (!priceItem) {
            QMessageBox::warning(this, "Error", QString("Row %1: missing price.").arg(i+1)); return;
        }
        bool ok; priceItem->text().toInt(&ok);
        if (!ok) priceItem->data(Qt::EditRole).toInt(&ok);
        if (!ok) {
            QMessageBox::warning(this, "Error", QString("Row %1: price must be integer cents.").arg(i+1)); return;
        }
    }

    QFile outFile("prices.csv");
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not write prices.csv"); return;
    }
    QTextStream out(&outFile);
    for (int i = 0; i < table->rowCount(); i++) {
        QString name = table->item(i, 0)->text().trimmed();
        bool ok; int price = table->item(i, 1)->text().toInt(&ok);
        if (!ok) price = table->item(i, 1)->data(Qt::EditRole).toInt();
        out << name << "," << price << "\n";
    }
    outFile.close();
    clear_prices();
    add_prices();
    ui_update();
    statusBar()->showMessage("Prices updated.", 3000);
}
