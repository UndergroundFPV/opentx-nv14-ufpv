#include "gui/common/windows/windows.h"

class ModelSetupPage: public MenuPage {
  public:
    ModelSetupPage();

    void build(Window * window);

  protected:
    void updateInternalModuleWindow();
    void updateExternalModuleWindow();

    Window * internalModuleWindow = nullptr;
    Window * externalModuleWindow = nullptr;
};
