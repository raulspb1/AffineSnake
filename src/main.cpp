#include "snake.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    // QApplication управляет ресурсами приложения и координирует работу оконной системы
    QApplication app(argc, argv);

    Snake game;
    game.setWindowTitle("Snake — Affine Transformations");
    game.show();

    return app.exec();
}
