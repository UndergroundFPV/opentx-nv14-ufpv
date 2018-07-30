#include "gui/common/windows/windows.h"

class RadioSdManagerPage: public PageTab {
  public:
    RadioSdManagerPage();

    void build(Window * window) override;

  protected:
    void rebuild(Window * window);
};
