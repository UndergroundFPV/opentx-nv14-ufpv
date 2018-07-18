#include "gui/common/windows/windows.h"

class RadioCalibrationPage: public PageTab {
  public:
    RadioCalibrationPage();

    void build(Window * window) override;

  protected:
    TextButton * button = nullptr;
    void nextStep();
};
