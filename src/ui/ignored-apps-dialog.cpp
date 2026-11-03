/**
 * @file ignored-apps-dialog.cpp
 * @brief Implementation of IgnoredAppsDialog.
 */

#include "ignored-apps-dialog.hpp"
#include "localization-helper.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

namespace AudioRouter {

IgnoredAppsDialog::IgnoredAppsDialog(const std::vector<std::string>& ignoredApps, QWidget *parent)
    : QDialog(parent), m_apps(ignoredApps)
{
    setWindowTitle(QString::fromStdString(loc("IgnoredAppsTitle", "Ignored Applications")));
    setModal(true);
    resize(420, 380);
    setStyleSheet(
        "QDialog {"
        "  background-color: #1e1f22;"
        "  color: #dcddde;"
        "  font-family: 'Segoe UI', sans-serif;"
        "}"
    );

    setupUi();
    rebuildRows();
}

void IgnoredAppsDialog::updateAppsList(const std::vector<std::string>& apps) {
    m_apps = apps;
    rebuildRows();
}

void IgnoredAppsDialog::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(10);

    // 1. Cabeçalho com Ícone e Descrição
    auto *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(4);

    auto *titleLabel = new QLabel(QString::fromUtf8("🚫 ") + QString::fromStdString(loc("IgnoredAppsTitle", "Ignored Applications")), this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ffffff; border: none;");
    headerLayout->addWidget(titleLabel);

    auto *descLabel = new QLabel(QString::fromStdString(loc("IgnoredAppsSubtitle", "Applications in this list do not appear in Router and output directly to Windows without volume alterations.")), this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 11px; color: #949ba4; border: none; line-height: 14px;");
    headerLayout->addWidget(descLabel);

    rootLayout->addLayout(headerLayout);

    // Divisor
    auto *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #2f3136; border: none; max-height: 1px;");
    rootLayout->addWidget(line);

    // 2. Área de Rolagem para a lista de aplicativos ignorados
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background-color: #18191c; border: 1px solid #2f3136; border-radius: 6px; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #4f545c; border-radius: 3px; }"
    );

    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background-color: transparent; border: none;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(8, 8, 8, 8);
    m_listLayout->setSpacing(6);
    m_listLayout->addStretch();

    m_scrollArea->setWidget(m_listContainer);
    rootLayout->addWidget(m_scrollArea, 1);

    // 3. Rodapé com Botão Fechar
    auto *footerLayout = new QHBoxLayout();
    footerLayout->addStretch();

    auto *closeBtn = new QPushButton(QString::fromStdString(loc("Close", "Close")), this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedHeight(28);
    closeBtn->setFixedWidth(90);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #35373c;"
        "  color: #ffffff;"
        "  font-weight: 500;"
        "  border: 1px solid #4e5058;"
        "  border-radius: 4px;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover { background-color: #4e5058; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(closeBtn);

    rootLayout->addLayout(footerLayout);
}

void IgnoredAppsDialog::rebuildRows() {
    // Remove widgets anteriores (exceto o stretch final)
    while (m_listLayout->count() > 1) {
        QLayoutItem *item = m_listLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    if (m_apps.empty()) {
        auto *emptyLabel = new QLabel(QString::fromStdString(loc("NoIgnoredApps", "No ignored applications configured.")), m_listContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color: #72767d; font-size: 11px; padding: 20px; font-style: italic;");
        m_listLayout->insertWidget(0, emptyLabel);
        return;
    }

    QString restoreText = QString::fromStdString(loc("RestoreApp", "Restore"));

    for (size_t i = 0; i < m_apps.size(); ++i) {
        const QString appName = QString::fromStdString(m_apps[i]);

        auto *rowWidget = new QFrame(m_listContainer);
        rowWidget->setStyleSheet(
            "QFrame {"
            "  background-color: #2b2d31;"
            "  border: 1px solid #313338;"
            "  border-radius: 4px;"
            "}"
            "QFrame:hover {"
            "  border-color: #404249;"
            "}"
        );
        auto *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(8, 6, 8, 6);
        rowLayout->setSpacing(8);

        auto *nameLabel = new QLabel(QString::fromUtf8("🚫 ") + appName, rowWidget);
        nameLabel->setStyleSheet("font-size: 11px; font-weight: 500; color: #dcddde; border: none;");
        rowLayout->addWidget(nameLabel, 1);

        auto *restoreBtn = new QPushButton(QString::fromUtf8("↺ ") + restoreText, rowWidget);
        restoreBtn->setCursor(Qt::PointingHandCursor);
        restoreBtn->setFixedHeight(24);
        restoreBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #1e1f22;"
            "  color: #5865f2;"
            "  font-weight: 600;"
            "  border: 1px solid #5865f2;"
            "  border-radius: 3px;"
            "  padding: 2px 8px;"
            "  font-size: 10px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #5865f2;"
            "  color: #ffffff;"
            "}"
        );

        connect(restoreBtn, &QPushButton::clicked, this, [this, appName]() {
            emit appRestored(appName);
        });

        rowLayout->addWidget(restoreBtn, 0);

        m_listLayout->insertWidget(static_cast<int>(i), rowWidget);
    }
}

} // namespace AudioRouter
