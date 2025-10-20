#ifndef APPLY_CATEGORY_DIALOG_H
#define APPLY_CATEGORY_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QString>

class ApplyCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    ApplyCategoryDialog(const QString &categoryId, const QString &categoryName,
                       const QString &thumbnailFilename, bool multiScreenMode,
                       int screenCount, QWidget *parent = nullptr);

signals:
    void applyThumbnail(int screenIndex);
    void applyRandomFromCategory(int screenIndex);

private slots:
    void onApplyThumbnail();
    void onApplyRandom();

private:
    QString m_categoryId;
    QString m_categoryName;
    QString m_thumbnailFilename;
    bool m_multiScreenMode;

    QComboBox *screenCombo;
    QPushButton *applyThumbnailBtn;
    QPushButton *applyRandomBtn;
    QPushButton *closeBtn;
};

#endif // APPLY_CATEGORY_DIALOG_H
