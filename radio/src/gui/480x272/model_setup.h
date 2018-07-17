#include "gui/common/windows/windows.h"

class ModelSetupPage: public PageTab {
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
    Choice * failSafeChoice= nullptr;

    Button * externalModuleBind = nullptr;
    TextButton * switchWarn[NUM_SWITCHES] = {nullptr};
    Button * externalModuleRange = nullptr;
};
