#include "snake.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QInputDialog>
#include <QtMath>
#include <cmath>

Snake::Snake(QWidget *parent) : QWidget(parent) {
    setStyleSheet("background-color:black;");
    setFixedSize(FIELD_WIDTH, FIELD_HEIGHT);

    loadImages(); // Генерируем и загружаем текстуры
    initGame();   // Инициализируем игру
}

// 1. Метод загрузки изображений яблока, тела и головы змеи
void Snake::loadImages() {
    // Создаем в памяти прозрачный холст для красного яблока
    QImage appleImg(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    appleImg.fill(Qt::transparent);
    QPainter p1(&appleImg);
    p1.setRenderHint(QPainter::Antialiasing);
    p1.setBrush(Qt::red);
    p1.setPen(Qt::NoPen);
    p1.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    p1.end();
    appleImg.save("apple.png");

    // Создаем холст для желтой головы змейки
    QImage headImg(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    headImg.fill(Qt::transparent);
    QPainter p2(&headImg);
    p2.setRenderHint(QPainter::Antialiasing);
    p2.setBrush(Qt::yellow);
    p2.setPen(Qt::NoPen);
    p2.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    p2.end();
    headImg.save("head.png");

    // Создаем холст для зеленого сегмента тела змейки
    QImage dotImg(DOT_SIZE, DOT_SIZE, QImage::Format_ARGB32);
    dotImg.fill(Qt::transparent);
    QPainter p3(&dotImg);
    p3.setRenderHint(QPainter::Antialiasing);
    p3.setBrush(Qt::green);
    p3.setPen(Qt::NoPen);
    p3.drawEllipse(0, 0, DOT_SIZE, DOT_SIZE);
    p3.end();
    dotImg.save("dot.png");

    // Загружаем файлы с диска в объекты QImage
    apple.load("apple.png");
    head.load("head.png");
    dot.load("dot.png");
}

// 2. Метод инициализации игры: создание змеи, размещение яблока и запуск таймера
void Snake::initGame() {
    inGame = true;
    dots = 6;          // Стартовая длина змейки (6 плотных сегментов)
    moveAngle = 0.0;   // Начальный угол 0 радиан (движение строго вправо)

    // Размещаем змейку в центре игрового поля
    double startX = FIELD_WIDTH / 2.0;
    double startY = FIELD_HEIGHT / 2.0;

    for (int i = 0; i < dots; ++i) {
        x[i] = startX - i * STEP_SIZE;
        y[i] = startY;
    }

    locateApple();                // Размещаем первое яблоко
    timerId = startTimer(DELAY);  // Запускаем игровой цикл
}

// 3. Метод случайного размещения яблока на игровом поле
void Snake::locateApple() {
    int margin = DOT_SIZE * 2;
    apple_x = QRandomGenerator::global()->bounded(margin, FIELD_WIDTH - margin);
    apple_y = QRandomGenerator::global()->bounded(margin, FIELD_HEIGHT - margin);
}

// Вспомогательный метод: аффинное перемещение точки (coordX, coordY) на вектор (dx, dy)
// с использованием матрицы переноса 3x3 и однородных координат [x, y, 1]^T
void Snake::applyAffineTranslation(double &coordX, double &coordY, double dx, double dy) const {
    // Матрица аффинного перемещения T(dx, dy):
    // | 1.0  0.0  dx |
    // | 0.0  1.0  dy |
    // | 0.0  0.0  1.0 |
    const std::array<std::array<double, 3>, 3> translationMatrix = {{
        {1.0, 0.0, dx},
        {0.0, 1.0, dy},
        {0.0, 0.0, 1.0}
    }};

    // Вектор однородных координат точки
    const std::array<double, 3> pointVec = {coordX, coordY, 1.0};
    std::array<double, 3> resultVec = {0.0, 0.0, 0.0};

    // Умножение матрицы 3x3 на вектор-столбец 3x1
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            resultVec[row] += translationMatrix[row][col] * pointVec[col];
        }
    }

    // Обновляем координаты точки
    coordX = resultVec[0];
    coordY = resultVec[1];
}

// 4. Метод перемещения змейки с помощью матрицы аффинных преобразований
void Snake::move() {
    // Перемещаем части тела по цепочке (от хвоста к голове):
    // каждый сегмент смещается на позицию впереди идущего через матрицу переноса
    for (int i = dots - 1; i > 0; --i) {
        double dx = x[i - 1] - x[i];
        double dy = y[i - 1] - y[i];
        applyAffineTranslation(x[i], y[i], dx, dy);
    }

    // Вычисляем приращения координат головы для движения под углом moveAngle
    double headDx = STEP_SIZE * std::cos(moveAngle);
    double headDy = STEP_SIZE * std::sin(moveAngle);

    // Перемещаем голову змейки с использованием аффинной матрицы перемещения
    applyAffineTranslation(x[0], y[0], headDx, headDy);
}

// 5. Метод проверки столкновений со стенами и собственным телом
void Snake::checkCollision() {
    // Проверка выхода головы за границы игрового поля
    if (x[0] < 0.0 || x[0] >= FIELD_WIDTH - DOT_SIZE ||
        y[0] < 0.0 || y[0] >= FIELD_HEIGHT - DOT_SIZE) {
        inGame = false;
    }

    // Проверка столкновения головы с телом по расстоянию между центрами сегментов
    // Начинаем с индекса 10, чтобы плавные повороты головы не вызывали ложное столкновение с шеей
    for (int i = dots - 1; i >= 10; --i) {
        double distance = std::hypot(x[0] - x[i], y[0] - y[i]);
        if (distance < DOT_SIZE * 0.5) {
            inGame = false;
            break;
        }
    }

    // Если произошло столкновение — останавливаем таймер игры
    if (!inGame) {
        killTimer(timerId);
    }
}

// 6. Игровой цикл по таймеру
void Snake::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    if (inGame) {
        // Проверяем, съела ли змейка яблоко (расстояние между центрами меньше размера сегмента)
        double distToApple = std::hypot(x[0] - apple_x, y[0] - apple_y);
        if (distToApple < DOT_SIZE) {
            // Увеличиваем длину змейки на 3 сегмента для заметного роста и спавним новое яблоко
            for (int k = 0; k < 3 && dots < ALL_DOTS - 1; ++k) {
                x[dots] = x[dots - 1];
                y[dots] = y[dots - 1];
                dots++;
            }
            locateApple();
        }

        checkCollision(); // Проверяем столкновения с препятствиями
        move();           // Перемещаем змейку
    }

    repaint(); // Перерисовываем окно
}

// Отрисовка графики кадра
void Snake::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);

    if (inGame) {
        // Рисуем яблоко
        qp.drawImage(qRound(apple_x), qRound(apple_y), apple);

        // Рисуем змейку от хвоста к голове, чтобы голова всегда была поверх тела
        for (int i = dots - 1; i >= 0; --i) {
            if (i == 0) {
                qp.drawImage(qRound(x[i]), qRound(y[i]), head);
            } else {
                qp.drawImage(qRound(x[i]), qRound(y[i]), dot);
            }
        }

        // Выводим подсказку по управлению и текущий угол в градусах
        qp.setPen(Qt::gray);
        qp.setFont(QFont("Sans", 9));
        int degrees = qRound(qRadiansToDegrees(moveAngle)) % 360;
        qp.drawText(10, 18, QString("Угол: %1° | Стрелки: поворот | [A]: задать угол").arg(degrees));
    } else {
        // Экран завершения игры
        qp.setPen(Qt::white);
        qp.setFont(QFont("Courier", 14, QFont::Bold));
        qp.drawText(rect(), Qt::AlignCenter, "Game Over\nНажмите Space для рестарта");
    }
}

// Обработка нажатий клавиш пользователем
void Snake::keyPressEvent(QKeyEvent *event) {
    int key = event->key();

    if (inGame) {
        if (key == Qt::Key_Left) {
            // Поворот вектора движения против часовой стрелки
            moveAngle -= TURN_STEP;
        } else if (key == Qt::Key_Right) {
            // Поворот вектора движения по часовой стрелке
            moveAngle += TURN_STEP;
        } else if (key == Qt::Key_Up) {
            // Направление строго вверх (-90 градусов)
            moveAngle = -M_PI_2;
        } else if (key == Qt::Key_Down) {
            // Направление строго вниз (+90 градусов)
            moveAngle = M_PI_2;
        } else if (key == Qt::Key_A) {
            // Ввод произвольного угла в градусах через контрастное диалоговое окно
            killTimer(timerId);

            QInputDialog dialog(this);
            dialog.setWindowTitle("Задать угол движения");
            dialog.setLabelText("Введите угол в градусах (0 — вправо, 90 — вниз, 180 — влево, 270 — вверх):");
            dialog.setInputMode(QInputDialog::DoubleInput);
            dialog.setDoubleRange(-3600.0, 3600.0);
            dialog.setDoubleDecimals(1);
            dialog.setDoubleValue(qRadiansToDegrees(moveAngle));
            dialog.setStyleSheet("background-color: white; color: black;");

            if (dialog.exec() == QDialog::Accepted) {
                moveAngle = qDegreesToRadians(dialog.doubleValue());
            }

            timerId = startTimer(DELAY);
        }
    } else if (key == Qt::Key_Space || key == Qt::Key_Return) {
        // Перезапуск игры после проигрыша
        initGame();
    }

    QWidget::keyPressEvent(event);
}
