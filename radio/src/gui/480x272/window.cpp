#include "widgets.h"

void Window::drawVerticalScrollbar()
{
  ::drawVerticalScrollbar(x+width-3, y + 3, height - 6, -offsetY, innerHeight, height);
}
