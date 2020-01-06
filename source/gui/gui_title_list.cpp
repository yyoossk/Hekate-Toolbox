#include "gui_title_list.hpp"
#include "button.hpp"
#include "utils.hpp"
#include <memory>
#include <string>
#include <cstring>
#include "gui_override_key.hpp"
#include "gametitle.hpp"
#include "SimpleIniParser.hpp"
#include <cstdlib>

#define COLUMNS 6
#define ICON_SIZE 160
#define ICON_PADDING (ICON_SIZE + 14)
#define TEXT_BORDER 10
#define GRIDPOS ( ( SCREEN_WIDTH - (COLUMNS * ICON_PADDING) ) /2)

GuiTitleList::GuiTitleList() : Gui() {
  auto apps = DumpAllGames(WidthHeight{ICON_SIZE, ICON_SIZE});
  int buttonIndex = 0;
  int buttonSelection = 0;

  size_t xOffset = 0, yOffset = 0;

  for (auto &&app : apps) {

    new Button(GRIDPOS + xOffset, 194 + yOffset, ICON_SIZE, ICON_SIZE,

    [app, buttonIndex](Gui *gui, u16 x, u16 y, bool *isActivated){
      if (app.get() != nullptr && app->icon.get() != nullptr)
        gui->drawImage(x, y, ICON_SIZE, ICON_SIZE, app->icon.get(), ImageMode::IMAGE_MODE_RGBA32);
      else
        gui->drawTextAligned(fontHuge, x + ICON_SIZE/2, y + ICON_SIZE/2 + 36, currTheme.textColor, "\uE06B", ALIGNED_CENTER);
      if (Button::getSelectedIndex() == buttonIndex) {
        u32 textWidth, textHeight;
        gui->getTextDimensions(font20, app->name, &textWidth, &textHeight);
        gui->drawRectangled((x + ICON_SIZE/2 - textWidth/2) - TEXT_BORDER*2, (y - 32 - textHeight) - TEXT_BORDER, textWidth+ TEXT_BORDER*4, textHeight+ TEXT_BORDER*2, currTheme.overlayColor);
        gui->drawTextAligned(font20, x + ICON_SIZE/2, y - 32, currTheme.highlightTextColor, app->name, ALIGNED_CENTER);
      }
    },

    [&, app](u64 kdown, bool *isActivated){
      if (kdown & KEY_A) {

        //convert title id to a hex string
        char buffer[17];
        sprintf(buffer, "%016lx", app->application_id);

        //write title id to config
        simpleIniParser::Ini *ini = simpleIniParser::Ini::parseOrCreateFile(LOADER_INI);
        ini->findOrCreateSection(HBL_CONFIG, true, simpleIniParser::IniSectionType::Section)
          ->findOrCreateFirstOption(OverrideKey::getOverrideProgramString(GuiOverrideKey::g_keyType), "")
          ->value = buffer;

        ini->writeToFile(LOADER_INI);
        delete ini;
        Gui::g_nextGui = GUI_OVERRIDE_KEY;
      }
    },
    
      {
        (buttonIndex - COLUMNS),                                          //UP
        (buttonIndex + COLUMNS),                                          //DOWN
        (buttonIndex % COLUMNS) != 0            ? (buttonIndex - 1) : -1, //LEFT
        (buttonIndex % COLUMNS) != COLUMNS-1    ? (buttonIndex + 1) : -1, //RIGHT
      }
    
      //{ -1, -1, buttonIndex - 1, buttonIndex + 1 }
     , false, []() -> bool {return true;});

    if (app->application_id == selectedAppID) {
      buttonSelection = buttonIndex;
    }

    xOffset += ICON_PADDING;
    if (xOffset >= COLUMNS * ICON_PADDING) {
      xOffset = 0;
      yOffset += ICON_PADDING;
    }
    buttonIndex++;
  }

  Button::select(buttonSelection);
}

GuiTitleList::~GuiTitleList() {
  Button::g_buttons.clear();
}

void GuiTitleList::update() {
  Gui::update();
}

void GuiTitleList::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2 - Button::targetOffsetX, 150 - Button::targetOffsetY, currTheme.textColor, "Select the title you wish to override.", ALIGNED_CENTER);

  for(Button *btn : Button::g_buttons)
    btn->draw(this);

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 88, currTheme.backgroundColor);
  Gui::drawRectangle(0, Gui::g_framebuffer_height - 73, Gui::g_framebuffer_width, 73, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontIcons, 70, 68, currTheme.textColor, "\uE130", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 58, currTheme.textColor, "        Application override settings", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 25, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  Gui::endDraw();
}

void GuiTitleList::onInput(u32 kdown) {
  for(Button *btn : Button::g_buttons) {
    if (btn->isSelected())
      if (btn->onInput(kdown)) return;
  }

  if (kdown & KEY_B)
    Gui::g_nextGui = GUI_OVERRIDE_KEY;
}

void GuiTitleList::onTouch(touchPosition &touch) {
  for(Button *btn : Button::g_buttons) {
    btn->onTouch(touch);
  }
}

void GuiTitleList::onGesture(touchPosition &startPosition, touchPosition &endPosition) {

}