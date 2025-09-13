#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QEvent>
#include <QScrollArea>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPixmap>
#include <QFrame>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

class ModernWindow : public QWidget
{
    Q_OBJECT

public:
    ModernWindow(QWidget *parent = nullptr) : QWidget(parent), networkManager(new QNetworkAccessManager(this))
    {
        setWindowTitle("WallpaperIA - Gestionnaire de Fonds d'écran");
        setWindowIcon(QIcon("icon.png"));
        setMinimumSize(800, 600);
        resize(1200, 800);
        
        // Désactiver le bouton fermer de la fenêtre
        setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
        
        setupUI();
        applyModernStyle();
        setupSystemTray();
        loadCategories();
    }

private:
    void setupUI()
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(15);
        
        // Titre
        QLabel *titleLabel = new QLabel("Catégories de Fonds d'écran");
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Zone scrollable pour les catégories
        scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setObjectName("scrollArea");
        
        // Widget contenant la grille des catégories
        categoriesWidget = new QWidget();
        categoriesLayout = new QGridLayout(categoriesWidget);
        categoriesLayout->setSpacing(15);
        categoriesLayout->setContentsMargins(10, 10, 10, 10);
        
        scrollArea->setWidget(categoriesWidget);
        mainLayout->addWidget(scrollArea);
    }
    
    void applyModernStyle()
    {
        setStyleSheet(R"(
            QWidget {
                background-color: #2b2b2b;
                color: #ffffff;
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 10pt;
            }
            
            #titleLabel {
                font-size: 18pt;
                font-weight: bold;
                color: #ffffff;
                margin-bottom: 10px;
            }
            
            #scrollArea {
                border: 1px solid #555;
                border-radius: 8px;
                background-color: #333;
            }
            
            #categoryFrame {
                background-color: #404040;
                border: 1px solid #555;
                border-radius: 12px;
                padding: 15px;
                margin: 5px;
            }
            
            #categoryFrame:hover {
                background-color: #4a4a4a;
                border-color: #777;
            }
            
            #categoryName {
                font-size: 12pt;
                font-weight: bold;
                color: #ffffff;
                text-align: center;
            }
            
            #thumbnailLabel {
                border: 2px solid #666;
                border-radius: 8px;
                background-color: #555;
            }
        )");
    }
    
    void setupSystemTray()
    {
        // Créer l'icône du system tray
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon("icon.png"));
        trayIcon->setToolTip("WallpaperIA");
        
        // Créer le menu contextuel
        trayMenu = new QMenu(this);
        
        QAction *showAction = new QAction("Afficher", this);
        QAction *quitAction = new QAction("Quitter", this);
        
        trayMenu->addAction(showAction);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAction);
        
        trayIcon->setContextMenu(trayMenu);
        
        // Connecter les signaux
        connect(showAction, &QAction::triggered, this, &ModernWindow::restoreWindow);
        connect(quitAction, &QAction::triggered, this, &ModernWindow::quitApplication);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &ModernWindow::onTrayIconActivated);
        
        // Afficher l'icône du tray
        trayIcon->show();
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::WindowStateChange) {
            if (isMinimized()) {
                // Masquer la fenêtre quand elle est minimisée
                hide();
                event->ignore();
                return;
            }
        }
        QWidget::changeEvent(event);
    }
    
    void closeEvent(QCloseEvent *event) override
    {
        // Minimiser vers le tray au lieu de fermer
        hide();
        event->ignore();
    }

    void loadCategories()
    {
        QNetworkRequest request(QUrl("http://localhost:8080/WallpaperIA/api/categories"));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                
                if (obj["success"].toBool()) {
                    QJsonArray categories = obj["data"].toArray();
                    displayCategories(categories);
                }
            }
            reply->deleteLater();
        });
    }
    
    void displayCategories(const QJsonArray &categories)
    {
        int row = 0, col = 0;
        const int maxCols = 3;
        
        for (const QJsonValue &value : categories) {
            QJsonObject category = value.toObject();
            QString categoryName = category["name"].toString();
            QString categoryId = category["id"].toString();
            
            createCategoryWidget(categoryName, categoryId, row, col);
            
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }
    
    void createCategoryWidget(const QString &name, const QString &id, int row, int col)
    {
        QFrame *categoryFrame = new QFrame();
        categoryFrame->setObjectName("categoryFrame");
        categoryFrame->setFixedSize(250, 200);
        categoryFrame->setCursor(Qt::PointingHandCursor);
        
        QVBoxLayout *frameLayout = new QVBoxLayout(categoryFrame);
        frameLayout->setSpacing(10);
        
        // Label pour la miniature (placeholder pour l'instant)
        QLabel *thumbnailLabel = new QLabel();
        thumbnailLabel->setObjectName("thumbnailLabel");
        thumbnailLabel->setFixedSize(200, 120);
        thumbnailLabel->setAlignment(Qt::AlignCenter);
        thumbnailLabel->setText("Miniature\n" + name);
        thumbnailLabel->setWordWrap(true);
        
        // Label pour le nom de la catégorie
        QLabel *nameLabel = new QLabel(name);
        nameLabel->setObjectName("categoryName");
        nameLabel->setWordWrap(true);
        
        frameLayout->addWidget(thumbnailLabel);
        frameLayout->addWidget(nameLabel);
        
        categoriesLayout->addWidget(categoryFrame, row, col);
        
        // Charger la première image de la catégorie comme miniature
        loadCategoryThumbnail(id, thumbnailLabel);
    }
    
    void loadCategoryThumbnail(const QString &categoryId, QLabel *thumbnailLabel)
    {
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/wallpapers?category=%1").arg(categoryId)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel]() {
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                
                if (obj["success"].toBool()) {
                    QJsonArray wallpapers = obj["data"].toArray();
                    if (!wallpapers.isEmpty()) {
                        QString filename = wallpapers[0].toObject()["filename"].toString();
                        loadThumbnailImage(filename, thumbnailLabel);
                    }
                }
            }
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImage(const QString &filename, QLabel *thumbnailLabel)
    {
        // Utiliser l'endpoint /mini/ pour les miniatures optimisées (90x50 JPG)
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/mini/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel, filename]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Les miniatures sont déjà optimisées (90x50), on les scale juste à la taille du label
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText(""); // Enlever le texte placeholder
                }
            } else {
                // En cas d'erreur avec /mini/, fallback vers l'ancienne méthode
                loadThumbnailImageFallback(filename, thumbnailLabel);
            }
            reply->deleteLater();
        });
    }
    
    void loadThumbnailImageFallback(const QString &filename, QLabel *thumbnailLabel)
    {
        // Méthode de fallback avec l'image complète (pour compatibilité)
        QNetworkRequest request(QUrl(QString("http://localhost:8080/WallpaperIA/api/get/%1").arg(filename)));
        QNetworkReply *reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, [this, reply, thumbnailLabel]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray imageData = reply->readAll();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    // Redimensionner l'image pour la miniature
                    QPixmap scaledPixmap = pixmap.scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    thumbnailLabel->setPixmap(scaledPixmap);
                    thumbnailLabel->setText(""); // Enlever le texte placeholder
                }
            }
            reply->deleteLater();
        });
    }

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            if (isVisible()) {
                hide();
            } else {
                restoreWindow();
            }
        }
    }
    
    void restoreWindow()
    {
        // Restaurer la fenêtre et la mettre au premier plan
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
        raise();
        activateWindow();
        setFocus();
        // Forcer la fenêtre à être au premier plan sur Windows
        #ifdef Q_OS_WIN
        SetForegroundWindow((HWND)winId());
        #endif
    }
    
    void quitApplication()
    {
        // Supprimer l'icône du tray avant de fermer
        if (trayIcon) {
            trayIcon->hide();
        }
        QApplication::quit();
    }

private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QNetworkAccessManager *networkManager;
    QScrollArea *scrollArea;
    QWidget *categoriesWidget;
    QGridLayout *categoriesLayout;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ModernWindow window;
    window.show();
    
    return app.exec();
}

#include "main.moc"