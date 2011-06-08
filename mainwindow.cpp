#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <KFileItem>
#include <KFileItemList>
#include <KIO/PreviewJob>
#include <KApplication>
#include <KCmdLineArgs>
#include <KMimeType>


#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/PendingReady>


#include "accounts-model.h"
#include "flat-model-proxy.h"
#include "account-filter-model.h"


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    Tp::registerTypes();

    ui->setupUi(this);

    qDebug() << KApplication::arguments();

    KUrl filePath (KCmdLineArgs::parsedArgs()->arg(0));
    ui->filePreview->showPreview(filePath);
    ui->fileNameLabel->setText(filePath.fileName());


    Tp::AccountFactoryPtr  accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                       Tp::Features() << Tp::Account::FeatureCore
                                                                       << Tp::Account::FeatureAvatar
                                                                       << Tp::Account::FeatureProtocolInfo
                                                                       << Tp::Account::FeatureProfile);

    Tp::ConnectionFactoryPtr connectionFactory = Tp::ConnectionFactory::create(QDBusConnection::sessionBus(),
                                                                               Tp::Features() << Tp::Connection::FeatureCore
                                                                               << Tp::Connection::FeatureRosterGroups
                                                                               << Tp::Connection::FeatureRoster
                                                                               << Tp::Connection::FeatureSelfContact);

    Tp::ContactFactoryPtr contactFactory = Tp::ContactFactory::create(Tp::Features()  << Tp::Contact::FeatureAlias
                                                                      << Tp::Contact::FeatureAvatarData
                                                                      << Tp::Contact::FeatureSimplePresence
                                                                      << Tp::Contact::FeatureCapabilities);

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());

    m_accountManager = Tp::AccountManager::create(QDBusConnection::sessionBus(),
                                                  accountFactory,
                                                  connectionFactory,
                                                  channelFactory,
                                                  contactFactory);


    connect(m_accountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)), SLOT(onAccountManagerReady()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAccountManagerReady()
{
    AccountsModel *model = new AccountsModel(m_accountManager, this);
    AccountFilterModel *filterModel = new AccountFilterModel(this);
    filterModel->setSourceModel(model);
    filterModel->filterOfflineUsers(true);
    FlatModelProxy *flatProxyModel = new FlatModelProxy(filterModel);

    ui->listView->setModel(flatProxyModel);
}
