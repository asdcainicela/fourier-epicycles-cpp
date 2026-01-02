# Fourier Epicycles Animation

Genera animaciones de epiciclos de Fourier al estilo Manim a partir de imágenes.

![Demo](imagen.png)

## 🎯 ¿Qué hace?

1. Extrae el contorno de una imagen (silueta)
2. Calcula la Transformada Discreta de Fourier (DFT) usando KissFFT
3. Genera un video con rendering de alta calidad (Cairo) o estándar (OpenCV)

---

## 📦 Requisitos

### Windows

#### 1. Instalar CMake
```powershell
winget install Kitware.CMake
```
O descarga desde: https://cmake.org/download/

> ⚠️ **Importante:** Después de instalar, cierra y abre PowerShell para que reconozca el comando `cmake`.

#### 2. Instalar Visual Studio Build Tools
```powershell
winget install Microsoft.VisualStudio.2022.BuildTools
```
Durante la instalación, selecciona **"Desktop development with C++"**

Alternativa: Instala [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/)

#### 3. Instalar vcpkg (gestor de dependencias)
```powershell
# Clonar vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Integrar con Visual Studio/CMake
.\vcpkg integrate install
```

#### 4. Instalar dependencias con vcpkg
```powershell
cd C:\vcpkg

# OpenCV (obligatorio)
.\vcpkg install opencv4:x64-windows

# Cairo (opcional, para mejor calidad de rendering)
.\vcpkg install cairo:x64-windows
```

> 💡 **Nota:** Cairo es opcional. Sin él, el programa usará OpenCV para renderizar (funciona igual, pero con menos antialiasing).

---

## 🔨 Compilación

### Paso 1: Clonar el repositorio
```powershell
git clone --recursive https://github.com/asdcainicela/fourier-epicycles-cpp.git
cd fourier-epicycles-cpp
```

> **Nota:** El flag `--recursive` descarga los submodules (`dmilos/color` y `kissfft`)

Si ya clonaste sin `--recursive`:
```powershell
git submodule update --init --recursive
```

### Paso 2: Crear directorio de build
```powershell
cd fourier_animation
mkdir build
cd build
```

### Paso 3: Configurar y Compilar

Tienes **dos opciones** para compilar:

#### Opción A: Usar Developer Command Prompt (Recomendado)

1. Abre **"Developer Command Prompt for VS 2022"** desde el menú Inicio
2. Navega al directorio de build:
   ```cmd
   cd "C:\ruta\a\fourier-epicycles-cpp\fourier_animation\build"
   ```
3. Configura con CMake:
   ```cmd
   cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```
4. Compila:
   ```cmd
   ninja
   ```

#### Opción B: Desde PowerShell (con wrapper de entorno)

Si prefieres usar PowerShell sin abrir Developer Command Prompt, usa este comando que configura el entorno automáticamente:

```powershell
# Configurar CMake
cmd /c "`"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat`" && cd /d `"$PWD`" && cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Compilar
cmd /c "`"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat`" && cd /d `"$PWD`" && ninja"
```

> ⚠️ **Nota:** Si tienes Visual Studio Community/Professional en lugar de Build Tools, la ruta sería:
> `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat`

### Salida esperada de CMake

Deberías ver algo como:
```
-- OpenCV version: 4.x.x
-- Cairo found via vcpkg
-- === Fourier Animation Build Configuration ===
-- Build type:     Release
-- OpenCV:         4.x.x
-- Cairo:          TRUE
-- ==============================================
```

---

## 🚀 Uso

```powershell
.\Release\fourier_animation.exe <imagen> [opciones]
```

### Ejemplos

```powershell
# Básico
.\Release\fourier_animation.exe mi_imagen.png

# Con opciones
.\Release\fourier_animation.exe logo.png -o video.mp4 -n 200 -f 900

# Alta calidad (más epiciclos, más resolución)
.\Release\fourier_animation.exe silueta.png -n 500 -w 3840 -h 2160 --fps 60
```

### Opciones disponibles

| Opción | Descripción | Default |
|--------|-------------|---------|
| `-o, --output <path>` | Ruta del video de salida | `fourier_output.mp4` |
| `-n, --circles <num>` | Número de epiciclos | `100` |
| `-f, --frames <num>` | Total de frames | `600` |
| `--fps <num>` | Frames por segundo | `60` |
| `-w, --width <num>` | Ancho del video | `1920` |
| `-h, --height <num>` | Alto del video | `1080` |
| `--samples <num>` | Puntos del contorno | `500` |
| `--no-circles` | Ocultar círculos | - |
| `--no-vectors` | Ocultar vectores | - |
| `--no-path` | Ocultar trazo | - |
| `--cpu` | Forzar encoding por CPU | - |

---

## 📁 Estructura del proyecto

```
fourier-epicycles-cpp/
├── external/
│   ├── color/              # Biblioteca de colores (submodule)
│   └── kissfft/            # FFT library (submodule)
├── fourier_animation/
│   ├── include/
│   │   ├── fourier.hpp
│   │   ├── contour_extractor.hpp
│   │   ├── animation.hpp
│   │   └── video_writer.hpp
│   ├── src/
│   │   ├── main.cpp
│   │   ├── fourier.cpp
│   │   ├── contour_extractor.cpp
│   │   ├── animation.cpp
│   │   └── video_writer.cpp
│   └── CMakeLists.txt
└── README.md
```

---

## 📚 Bibliotecas externas

| Biblioteca | Tipo | Descripción |
|------------|------|-------------|
| **OpenCV** | vcpkg | Extracción de contornos, video I/O |
| **Cairo** | vcpkg (opcional) | Rendering 2D de alta calidad |
| **KissFFT** | submodule | Transformada de Fourier |
| **dmilos/color** | submodule | Manejo de colores |

### Ejemplo de uso de colores
```cpp
#include <color/color.hpp>

// Usar constantes de color
::color::rgb<std::uint8_t> rojo = ::color::constant::red_t{};
::color::rgb<std::uint8_t> turquesa = ::color::constant::turquoise_t{};

// Convertir a cv::Scalar para OpenCV (BGR)
cv::Scalar scalar(rojo[2], rojo[1], rojo[0]);
```

---

## 🐛 Solución de problemas

### "cmake : El término 'cmake' no se reconoce"
Instala CMake y **reinicia PowerShell**:
```powershell
winget install Kitware.CMake
# Cierra y abre PowerShell
```

### "Running 'nmake' '-?' failed" o "CMAKE_CXX_COMPILER not set"
Este error ocurre cuando ejecutas CMake desde PowerShell sin el entorno de Visual Studio configurado. **Soluciones:**

1. **Usar Developer Command Prompt** (ver Paso 3, Opción A), o
2. **Usar el wrapper de entorno** (ver Paso 3, Opción B)

### "Could not find any instance of Visual Studio"
CMake no encuentra Visual Studio. Verifica:
1. Tienes instalado Visual Studio Build Tools o Visual Studio Community
2. Instalaste el componente **"Desktop development with C++"**
3. Si aún falla, limpia la caché de CMake y usa el generador Ninja:
```powershell
Remove-Item -Recurse -Force CMakeCache.txt, CMakeFiles
# Luego usa la Opción B del Paso 3
```

### "Error: generator does not match the generator used previously"
Limpia la caché de CMake antes de cambiar el generador:
```powershell
Remove-Item -Recurse -Force CMakeCache.txt, CMakeFiles
```

### "OpenCV not found"
Asegúrate de usar el toolchain de vcpkg:
```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### "CUDA not found" (warning)
Es solo una advertencia. CUDA es opcional para aceleración de hardware.

### Los submodules no se descargaron
```powershell
git submodule update --init --recursive
```

### El video no se genera
Asegúrate de tener codecs instalados. Instala [K-Lite Codec Pack](https://codecguide.com/download_kl.htm) si es necesario.

### Cairo no se detecta
Verifica que instalaste Cairo con vcpkg:
```powershell
C:\vcpkg\vcpkg install cairo:x64-windows
```

---

## 📄 Licencia

MIT License

---

## 🙏 Créditos

- Inspirado en [3Blue1Brown](https://www.youtube.com/watch?v=r6sGWTCMz2k)
- FFT: [KissFFT](https://github.com/mborgerding/kissfft)
- Rendering: [Cairo](https://www.cairographics.org/) / OpenCV
- Biblioteca de colores: [dmilos/color](https://github.com/dmilos/color)
- Visualización: OpenCV
