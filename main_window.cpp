#include "main_window.h"
#include "bot_factory/bot_factory.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
{
    BotFactory* f = new BotFactory(this);
}
