#pragma once

#include <QFrame>
#include <QMapboxGL>
#include <QStackedLayout>

class OnroadSettingsPanel : public QFrame {
  Q_OBJECT

public:
  explicit OnroadSettingsPanel(QWidget *parent = nullptr);

signals:
  void onroadSettingsPanelRequested();

public slots:
  void toggleOnroadSettings();

private:
  QStackedLayout *content_stack;
};
