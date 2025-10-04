#!/usr/bin/env python3
from PIL import Image
import numpy as np
import sys

try:
    # Try PPM first (default format), then PNG
    try:
        img = Image.open('triangle_splatting_frame.ppm')
        format_name = "PPM"
    except FileNotFoundError:
        img = Image.open('triangle_splatting_frame.png')
        format_name = "PNG"
    
    arr = np.array(img)
    
    print(f"📊 {format_name} АНАЛИЗ:")
    print(f"  Размер: {arr.shape[1]}x{arr.shape[0]}")
    print(f"  Каналы: {arr.shape[2] if len(arr.shape) > 2 else 1}")
    print(f"  Min значение: {arr.min()}")
    print(f"  Max значение: {arr.max()}")
    print(f"  Среднее: {arr.mean():.2f}")
    
    # Уникальные цвета
    if len(arr.shape) == 3:
        unique = np.unique(arr.reshape(-1, arr.shape[2]), axis=0)
        print(f"  Уникальных цветов: {len(unique)}")
        
        if len(unique) <= 10:
            print(f"\n  Цвета (RGB):")
            for i, color in enumerate(unique[:10]):
                print(f"    {i+1}. {color}")
    
    # Проверка на черный экран
    if arr.max() == 0:
        print("\n❌ ПРОБЛЕМА: Полностью черный экран!")
    elif arr.mean() < 10:
        print(f"\n⚠️  ПРОБЛЕМА: Почти черный экран (среднее={arr.mean():.2f})")
    else:
        print(f"\n✅ OK: Есть видимый контент (среднее={arr.mean():.2f})")
    
except Exception as e:
    print(f"❌ Ошибка: {e}")
    sys.exit(1)
