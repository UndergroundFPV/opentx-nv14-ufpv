#include "gui/common/windows/windows.h"

class RadioCalibrationPage: public PageTab {
  public:
    RadioCalibrationPage();

    virtual void build(Window * window) override;

    virtual void checkEvents() override;

  protected:
    TextButton * button = nullptr;
    void nextStep();
};
