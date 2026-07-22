#include "ui/mainwindow.h"

viewer::ImagePreviewDialog::ImagePreviewDialog(const QString& image_path, QWidget *parent = nullptr) : QDialog(parent) {
    setWindowTitle("Предпросмотр изображения");
    setFixedSize(800, 600); // Фиксированный размер для аккуратного отображения
    setModal(true);

    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(15, 15, 15, 15);
    main_layout->setSpacing(10);

    auto *label_image = new QLabel(this);
    label_image->setAlignment(Qt::AlignCenter);
    label_image->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

    QPixmap pixmap(image_path);
    if (!pixmap.isNull()) {
        // Масштабируем картинку, чтобы она красиво вписалась в окно предпросмотра
        label_image->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::FastTransformation));
    } else {
        label_image->setText("Ошибка загрузки\nизображения");
    }

    // Позволяем картинке занять всё свободное пространство сверху
    main_layout->addWidget(label_image, 1);

    // --- Область кнопок ---
    auto *btn_layout = new QHBoxLayout();
    auto *btn_cancel = new QPushButton("Отменить", this);
    auto *btn_save = new QPushButton("Сохранить", this);

    btn_cancel->setMinimumHeight(35);
    btn_save->setMinimumHeight(35);
    // Сделаем кнопку сохранения более акцентной
    btn_save->setStyleSheet("font-weight: bold;");

    btn_layout->addWidget(btn_cancel);
    btn_layout->addWidget(btn_save);
    main_layout->addLayout(btn_layout);

    // Связываем кнопки с возвращаемыми состояниями диалога
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btn_save, &QPushButton::clicked, this, &QDialog::accept);
}