/**
 * @file ignored-apps-dialog.hpp
 * @brief Modal dialog displaying the list of ignored applications,
 *        allowing users to restore them to active audio routing.
 */

#pragma once

#include <QDialog>
#include <QString>
#include <vector>
#include <string>

class QVBoxLayout;
class QScrollArea;

namespace AudioRouter {

class IgnoredAppsDialog : public QDialog {
    Q_OBJECT

public:
    explicit IgnoredAppsDialog(const std::vector<std::string>& ignoredApps, QWidget *parent = nullptr);
    ~IgnoredAppsDialog() override = default;

    void updateAppsList(const std::vector<std::string>& apps);

signals:
    void appRestored(const QString& appName);

private:
    void setupUi();
    void rebuildRows();

    std::vector<std::string> m_apps;
    QWidget *m_listContainer = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
};

} // namespace AudioRouter
