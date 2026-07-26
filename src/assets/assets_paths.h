#pragma once

/** @file assets_paths.h
 *  @brief Пути к директориям с ресурсами движка.
 *
 *  Определяет имена папок (относительно корня ресурсов),
 *  в которых хранятся шейдеры, текстуры, шрифты, макеты,
 *  звуки, модели и пост-эффекты.
 */

#include <string>

/** @brief Папка с шейдерами */
inline const std::string SHADERS_FOLDER = "shaders";
/** @brief Папка с текстурами */
inline const std::string TEXTURES_FOLDER = "textures";
/** @brief Папка со шрифтами */
inline const std::string FONTS_FOLDER = "fonts";
/** @brief Папка с файлами макетов (layout) */
inline const std::string LAYOUTS_FOLDER = "layouts";
/** @brief Папка со звуковыми файлами */
inline const std::string SOUNDS_FOLDER = "sounds";
/** @brief Папка с 3D-моделями */
inline const std::string MODELS_FOLDER = "models";
/** @brief Папка с пост-эффектами (внутри папки шейдеров) */
inline const std::string POST_EFFECTS_FOLDER = "shaders/effects";
/** @brief Имя шрифта по умолчанию */
inline const std::string FONT_DEFAULT = "normal";
