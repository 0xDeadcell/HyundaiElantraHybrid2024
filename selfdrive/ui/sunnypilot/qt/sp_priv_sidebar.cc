#include "selfdrive/ui/sunnypilot/qt/sp_priv_sidebar.h"

#include <cmath>

#include <QMouseEvent>

#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/sunnypilot/qt/sp_priv_util.h"
#include "common/params.h"


SidebarSP::SidebarSP(QWidget *parent) : Sidebar(parent) {
  // Because I know that stock sidebar makes this connection, I will disconnect it and connect it to the new updateState function
  QObject::disconnect(uiState(), &UIState::uiUpdate, this, &Sidebar::updateState);
  QObject::connect(uiStateSP(), &UIStateSP::uiUpdate, this, &SidebarSP::updateState);
}

void SidebarSP::updateState(const UIStateSP &s) {
  if (!isVisible()) return;
  Sidebar::updateState(s);
  auto &sm = *(s.sm);
  auto deviceState = sm["deviceState"].getDeviceState();

  if (sm.frame % UI_FREQ == 0) { // Update every 1 Hz
    switch (s.scene.sidebar_temp_options) {
    case 0:
      break;
    case 1:
      sidebar_temp = QString::number((int)deviceState.getMemoryTempC());
      break;
    case 2: {
        const auto& cpu_temp_list = deviceState.getCpuTempC();
        float max_cpu_temp = std::numeric_limits<float>::lowest();

        for (const float& temp : cpu_temp_list) {
          max_cpu_temp = std::max(max_cpu_temp, temp);
        }

        if (max_cpu_temp >= 0) {
          sidebar_temp = QString::number(std::nearbyint(max_cpu_temp));
        }
        break;
    }
    case 3: {
        const auto& gpu_temp_list = deviceState.getGpuTempC();
        float max_gpu_temp = std::numeric_limits<float>::lowest();

        for (const float& temp : gpu_temp_list) {
          max_gpu_temp = std::max(max_gpu_temp, temp);
        }

        if (max_gpu_temp >= 0) {
          sidebar_temp = QString::number(std::nearbyint(max_gpu_temp));
        }
        break;
    }
    case 4:
      sidebar_temp = QString::number((int)deviceState.getMaxTempC());
      break;
    default:
      break;
    }

    setProperty("sidebarTemp", sidebar_temp + "°C");
  }

  bool show_sidebar_temp = s.scene.sidebar_temp_options != 0;
  ItemStatus tempStatus = {{tr("TEMP"), show_sidebar_temp ? sidebar_temp_str : tr("HIGH")}, danger_color};
  auto ts = deviceState.getThermalStatus();
  if (ts == cereal::DeviceState::ThermalStatus::GREEN) {
    tempStatus = {{tr("TEMP"), show_sidebar_temp ? sidebar_temp_str : tr("GOOD")}, good_color};
  } else if (ts == cereal::DeviceState::ThermalStatus::YELLOW) {
    tempStatus = {{tr("TEMP"), show_sidebar_temp ? sidebar_temp_str : tr("OK")}, warning_color};
  }
  setProperty("tempStatus", QVariant::fromValue(tempStatus));

  ItemStatus sunnylinkStatus;
  auto sl_dongle_id = getSunnylinkDongleId();
  auto last_sunnylink_ping_str = params.get("LastSunnylinkPingTime");
  auto last_sunnylink_ping = std::stoull(last_sunnylink_ping_str.empty() ? "0" : last_sunnylink_ping_str);
  auto elapsed_sunnylink_ping = nanos_since_boot() - last_sunnylink_ping;
  auto sunnylink_enabled = params.getBool("SunnylinkEnabled");

  QString status = tr("DISABLED");
  QColor color = disabled_color;

  if (sunnylink_enabled && last_sunnylink_ping == 0) {
    // If sunnylink is enabled, but we don't have a dongle id, and we haven't received a ping yet, we are registering
    status = sl_dongle_id.has_value() ? tr("OFFLINE") : tr("REGIST...");
    color = sl_dongle_id.has_value() ? warning_color : progress_color;
  } else if (sunnylink_enabled) {
    // If sunnylink is enabled, we are considered online if we have received a ping in the last 80 seconds, else error.
    status = elapsed_sunnylink_ping < 80000000000ULL ? tr("ONLINE") : tr("ERROR");
    color = elapsed_sunnylink_ping < 80000000000ULL ? good_color : danger_color;
  }
  sunnylinkStatus = ItemStatus{{tr("SUNNYLINK"), status}, color };
  setProperty("sunnylinkStatus", QVariant::fromValue(sunnylinkStatus));
}


void SidebarSP::DrawSidebar(QPainter &p){
  Sidebar::DrawSidebar(p);
  // metrics
  drawMetric(p, temp_status.first, temp_status.second, 310);
  drawMetric(p, panda_status.first, panda_status.second, 440);
  drawMetric(p, connect_status.first, connect_status.second, 570);
  drawMetric(p, sunnylink_status.first, sunnylink_status.second, 700);
}
