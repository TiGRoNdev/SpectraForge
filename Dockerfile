FROM ubuntu:20.04

# Установка зависимостей
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libgl1-mesa-dev \
    libglfw3-dev \
    libglew-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Создание рабочей директории
WORKDIR /app

# Копирование исходного кода
COPY . .

# Сборка проекта
RUN mkdir build && cd build && \
    cmake .. && \
    make -j4

# Запуск демо
CMD ["./build/Engine4D_Demo"]
