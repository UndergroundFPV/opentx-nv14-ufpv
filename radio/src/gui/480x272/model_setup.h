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

    Choice * internalModuleChoice = nullptr;
    Choice * externalModuleChoice = nullptr;

    Button * externalModuleBind = nullptr;
    Button * switchWarn[NUM_SWITCHES] = {nullptr};
    Button * externalModuleRange = nullptr;
};
