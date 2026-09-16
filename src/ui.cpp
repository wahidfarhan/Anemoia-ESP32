#include "ui.h"

#define UI_BG       0xDFE7
#define UI_BG_DARK  0xC9E4
#define UI_PANEL    0xF7FA
#define UI_PANEL2   0xE6F5
#define UI_BLUE     0x1BD3
#define UI_CYAN     0x1DEB
#define UI_GOLD     0xF6A0
#define UI_TEXT     0x0000
#define UI_MUTED    0x6B6D
#define UI_ACCENT   0x00C6
#define UI_LIME     0x5DE8
#define UI_RED      0xF800

UI::UI(TFT_eSPI* screen)
{
    this->screen = screen;
}

UI::~UI()
{
}

static void drawBackgroundPattern(TFT_eSPI* s)
{
    s->fillScreen(UI_BG);

    for (int y = 0; y < s->height(); y += 12)
    {
        for (int x = 0; x < s->width(); x += 12)
        {
            uint16_t c = (x + y) % 24 == 0 ? UI_BG_DARK : UI_BG;
            s->fillRect(x, y, 8, 8, c);
        }
    }

    s->fillRect(0, 0, s->width(), 56, UI_PANEL);
    s->fillRect(0, 56, s->width(), 2, UI_CYAN);
}

static void drawLogoHeader(TFT_eSPI* s)
{
    drawBackgroundPattern(s);
    s->setTextColor(UI_BLUE, UI_PANEL);
    s->setTextSize(2);
    s->setTextDatum(MC_DATUM);
    s->drawString("CodingVerse", s->width() / 2, 30, 2);
}

static void drawLightPanel(TFT_eSPI* s, int x, int y, int w, int h)
{
    s->fillRoundRect(x, y, w, h, 8, UI_PANEL);
    s->drawRoundRect(x, y, w, h, 8, UI_ACCENT);
}

Cartridge* UI::selectGame()
{
    unsigned int last_input_time = 0;
    constexpr unsigned int delay = 250;
    extern bool demo_mode_active;
    extern unsigned int demo_mode_roms_menu_timeout;
    max_items = (screen->height() - 120) / ITEM_HEIGHT;

    getNesFiles();
    drawLogoHeader(screen);

    const int size = files.size();
    while (true)
    {
        bool game_chosen = false;
        unsigned int now = millis();

        if (now - last_input_time > delay)
        {
            if (isDownPressed(CONTROLLER::Up))
            {
                selected--;
                if (selected < 0) selected = (size - 1);
                if (selected < scroll_offset) scroll_offset = selected;
                if (scroll_offset < 0) scroll_offset = 0;
                last_input_time = now;
            }

            if (isDownPressed(CONTROLLER::Down))
            {
                selected++;
                if (selected >= size) selected = 0;
                if (selected >= scroll_offset + max_items)
                    scroll_offset = selected - max_items + 1;
                if (scroll_offset < 0) scroll_offset = 0;
                if (scroll_offset > size - 1) scroll_offset = size - 1;
                last_input_time = now;
            }

            if (isDownPressed(CONTROLLER::Left))
            {
                selected -= max_items;
                if (selected < 0) selected = 0;
                last_input_time = now;
            }

            if (isDownPressed(CONTROLLER::Right))
            {
                selected += max_items;
                if (selected > size - 1) selected = size - 1;
                last_input_time = now;
            }

            if (isDownPressed(CONTROLLER::Select))
            {
                settings.rom_backend = (settings.rom_backend + 1) % 2;
                saveSettings(&settings);
                last_input_time = now;
            }
        }

        if (isDownPressed(CONTROLLER::A) && (selected >= 0 && selected < size))
        {
            game_chosen = true;
        }

        if (runtime_config.demo_mode)
        {
            if (controllerRead()) demo_mode_active = false;
            if (demo_mode_active && (now - last_input_time) >= demo_mode_roms_menu_timeout)
            {
                selected = esp_random() % size;
                game_chosen = true;
            }
        }

        if (game_chosen)
        {
            if (runtime_config.backlight) { ledcWrite(TFT_BACKLIGHT_PIN, 0); }
            std::string game = "/" + files[selected];
            std::vector<std::string>().swap(files);
            ROMBackend backend = (ROMBackend)settings.rom_backend;
            screen->fillScreen(UI_BG);
            return new Cartridge(game.c_str(), backend);
        }

        drawFileList();
        drawRomMode();
        drawBars();
    }
}

void UI::getNesFiles()
{
    File root = SD.open("/");
    while (true)
    {
        File file = root.openNextFile();
        if (!file) break;
        if (!file.isDirectory())
        {
            std::string filename = file.name();
            if (filename.rfind(".nes") == filename.size() - 4) files.push_back(filename);
        }
        file.close();
    }
    root.close();
}

void UI::drawFileList()
{
    screen->fillRect(12, 72, screen->width() - 24, screen->height() - 120, UI_PANEL);

    const int size = files.size();
    for (int i = 0; i < max_items; i++)
    {
        int item = i + scroll_offset;
        if (item >= size) break;

        std::string file = files[item];
        int maxWidth = screen->width() - 100;
        while (screen->textWidth(file.c_str()) > maxWidth) { file.pop_back(); }
        if (file.size() < files[item].size()) { file.replace(file.size() - 3, 3, "..."); }

        const char* filename = file.c_str();
        int y = i * ITEM_HEIGHT + 90;
        if (item == selected)
        {
            screen->fillRoundRect(18, y - 2, screen->width() - 36, ITEM_HEIGHT + 4, 6, UI_CYAN);
            screen->setTextColor(UI_TEXT, UI_CYAN);
            screen->drawString(filename, 24, y, 2);
        }
        else
        {
            screen->setTextColor(UI_TEXT, UI_PANEL);
            screen->drawString(filename, 24, y, 2);
        }
    }
}

void UI::drawWindowBox(int x, int y, int w, int h)
{
    screen->fillRoundRect(x, y, w, h, 8, UI_PANEL);
    screen->drawRoundRect(x, y, w, h, 8, UI_ACCENT);
}

void UI::drawBars()
{
    screen->fillRect(0, screen->height() - 36, screen->width(), 36, UI_PANEL);
    screen->setTextColor(UI_TEXT, UI_PANEL);

    screen->setTextDatum(TL_DATUM);
    screen->drawString("Move", 12, screen->height() - 28, 2);
    screen->drawString("Select", 140, screen->height() - 28, 2);
    screen->drawString("A", 240, screen->height() - 28, 2);

    screen->setTextDatum(MC_DATUM);
    screen->setTextColor(UI_BLUE, UI_PANEL);
    screen->drawString("ROM List", screen->width() / 2, 58, 2);
}

void UI::pauseMenu(Nes* nes)
{
    __attribute__((used, section(".text"), aligned(64))) static const uint8_t padding[128] = { 0 };

    paused = true;
    int prev_select = 0;
    int select = 0;

    screen->fillScreen(UI_BG);
    drawLogoHeader(screen);
    drawWindowBox(20, 70, screen->width() - 40, screen->height() - 110);

    const char* items[] = { "Resume", "Settings", "Reset", "Quick Save", "Quick Load", "Exit" };
    int items_y[] = { 90, 112, 134, 156, 178, 200 };
    int num_items = 6;

    screen->fillRoundRect(30, 80, screen->width() - 60, 200, 10, UI_PANEL2);
    screen->fillRoundRect(30, 80, 100, 18, 4, UI_CYAN);

    for (int i = 0; i < num_items; i++)
    {
        int y = items_y[i];
        if (i == select)
            screen->fillRoundRect(30, y, screen->width() - 90, 14, 4, UI_CYAN);
        else
            screen->fillRect(30, y, screen->width() - 90, 14, UI_PANEL2);

        screen->setTextColor(UI_TEXT, (i == select) ? UI_CYAN : UI_PANEL2);
        screen->drawString(items[i], 40, y + 2, 2);
    }

    while (true)
    {
        if (isDownPressed(CONTROLLER::Up)) { select--; if (select < 0) select = num_items - 1; }
        if (isDownPressed(CONTROLLER::Down)) { select++; if (select > num_items - 1) select = 0; }
        if (isDownPressed(CONTROLLER::A))
        {
            if (select == 0) { paused = false; return; }
            if (select == 1) settingsMenu(nes);
            if (select == 2) { nes->reset(); paused = false; return; }
            if (select == 3) { nes->saveState(); paused = false; return; }
            if (select == 4) { nes->loadState(); paused = false; return; }
            if (select == 5) { ESP.restart(); }
        }
        delay(100);
    }
}

void UI::settingsMenu(Nes* nes)
{
    screen->fillScreen(UI_BG);
    drawLogoHeader(screen);

    int x = 20;
    int y = 80;
    int w = screen->width() - 40;
    int h = screen->height() - 120;

    drawWindowBox(x, y, w, h);

    const char* items[] = { "Volume", "Brightness", "Palette", "Back" };
    int items_y[] = { 100, 130, 160, 190 };
    int select = 0;

    while (true)
    {
        if (isDownPressed(CONTROLLER::Up)) { select--; if (select < 0) select = 3; }
        if (isDownPressed(CONTROLLER::Down)) { select++; if (select > 3) select = 0; }

        if (isDownPressed(CONTROLLER::A))
        {
            if (select == 3) { loadEmulatorSettings(nes); saveSettings(&settings); return; }
        }

        if (isDownPressed(CONTROLLER::Left))
        {
            if (select == 0) { if (settings.volume > 5) settings.volume -= 5; }
            if (select == 1) { if (settings.brightness > 10) settings.brightness -= 5; }
            if (select == 2) { if (settings.palette > 0) settings.palette--; }
        }

        if (isDownPressed(CONTROLLER::Right))
        {
            if (select == 0) { if (settings.volume < 95) settings.volume += 5; }
            if (select == 1) { if (settings.brightness < 95) settings.brightness += 5; }
            if (select == 2) { if (settings.palette < Ppu2C02::Palette::PaletteCount - 1) settings.palette++; }
        }

        screen->fillRect(x + 10, y + 10, w - 20, h - 20, UI_PANEL2);
        for (int i = 0; i < 4; i++)
        {
            int yy = items_y[i];
            if (i == select)
            {
                screen->fillRoundRect(x + 10, yy, w - 30, 18, 6, UI_CYAN);
                screen->setTextColor(UI_TEXT, UI_CYAN);
            }
            else
            {
                screen->fillRect(x + 10, yy, w - 30, 18, UI_PANEL2);
                screen->setTextColor(UI_TEXT, UI_PANEL2);
            }
            screen->drawString(items[i], x + 18, yy + 2, 2);
        }
        delay(80);
    }
}

void UI::initializeSettings()
{
    if (!SD.exists(\"/settings.bin\"))
    {
        Settings temp = { 100, 100, 0 };
        saveSettings(&temp);
    }
    loadSettings(&settings);
}

void UI::loadEmulatorSettings(Nes* nes)
{
    nes->setPalette(settings.palette);
    nes->setVolume(settings.volume);
}

void UI::restoreBrightness()
{
    if (runtime_config.backlight) setBrightness(settings.brightness);
}

void UI::setBrightness(int value)
{
    uint8_t pwm = ((value * 255) + 50) / 100;
    ledcWrite(TFT_BACKLIGHT_PIN, pwm);
}

void UI::saveSettings(const Settings* s)
{
    File f = SD.open(\"/settings.bin\", FILE_WRITE);
    if (!f) return;
    f.seek(0);
    f.write((uint8_t*)s, sizeof(*s));
    f.close();
}

void UI::loadSettings(Settings* s)
{
    File f = SD.open(\"/settings.bin\", FILE_READ);
    if (!f) return;
    if (f.size() != sizeof(Settings))
    {
        f.close();
        Settings temp = { 100, 100, 0 };
        saveSettings(&temp);
        *s = temp;
        f.close();
        return;
    }
    f.read((uint8_t*)s, sizeof(*s));
    f.close();
}

void UI::drawText(const char* text, const int16_t x, const int16_t y)
{
    screen->setTextColor(UI_TEXT, UI_PANEL);
    screen->setCursor(x, y);
    screen->print(text);
}

void UI::drawRomMode()
{
    const int16_t y = screen->height() - 12;
    const char* selectText = \"Select\";
    const char* mode1 = \" RAM mode\";
    const char* mode2 = \" Flash mode\";
    const char* currentMode = mode1;
    switch (settings.rom_backend)
    {
    case 0: currentMode = mode1; break;
    case 1: currentMode = mode2; break;
    default: currentMode = mode1; break;
    }

    screen->setTextColor(UI_ACCENT, UI_PANEL);
    screen->setCursor(0, y);
    screen->print(selectText);

    screen->setTextColor(UI_TEXT, UI_PANEL);
    screen->print(currentMode);
}
