#pragma once

#include <QWidget>
#include <QKeyEvent>
#include <QImage>
#include <array>

class Snake : public QWidget {
    Q_OBJECT

public:
    explicit Snake(QWidget *parent = nullptr);

protected:
    // Системные обработчики событий Qt
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    // Обязательные методы логики согласно ТЗ
    void loadImages();       // Генерация и загрузка текстур
    void initGame();         // Настройка стартовых параметров
    void locateApple();      // Случайное размещение яблока на поле
    void move();             // Перемещение змейки через аффинную матрицу
    void checkCollision();   // Проверка столкновений со стенами и хвостом

    // Метод аффинного преобразования (перемещение точки в однородных координатах 3x3)
    void applyAffineTranslation(double &coordX, double &coordY, double dx, double dy) const;

    // Константы игрового поля и параметров движения
    static constexpr int DOT_SIZE = 12;          // Размер сегмента и яблока (в пикселях)
    static constexpr int ALL_DOTS = 2500;        // Максимальная длина массива сегментов
    static constexpr int FIELD_WIDTH = 500;      // Ширина игрового окна
    static constexpr int FIELD_HEIGHT = 500;     // Высота игрового окна
    static constexpr int DELAY = 40;             // Интервал таймера в мс (плавная отрисовка)
    static constexpr double STEP_SIZE = 4.0;     // Шаг перемещения головы за один тик (в пикселях)
    static constexpr double TURN_STEP = 0.15;    // Шаг поворота угла при нажатии стрелок (в радианах)

    // Игровые переменные
    int timerId = 0;         // ID активного таймера
    int dots = 0;            // Текущая длина змейки (количество сегментов)
    bool inGame = true;      // Статус игры (true — идет игра, false — Game Over)

    double apple_x = 0.0;    // Координата X яблока
    double apple_y = 0.0;    // Координата Y яблока
    double moveAngle = 0.0;  // Текущий угол движения головы змеи (в радианах, 0 — вправо)

    // Массивы координат сегментов змейки с плавающей точкой для точного движения под углом
    double x[ALL_DOTS];
    double y[ALL_DOTS];

    // Текстуры игровых объектов
    QImage apple;
    QImage head;
    QImage dot;
};
