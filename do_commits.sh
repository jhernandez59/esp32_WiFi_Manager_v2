#!/bin/bash

echo "🚀 Realizando commits organizados..."

# Commit 1
git add .gitignore
git commit -m "chore: Mejorar .gitignore con seguridad y más casos

- Agregar protección para archivos de credenciales
- Agregar más build artifacts de ESP32
- Agregar soporte para Python/scripts
- Prevenir commit accidental de secrets"

echo "✅ Commit 1/3 completado"

# Commit 2
git add docs/ setup_docs.sh
git commit -m "docs: Crear estructura profesional de documentación

- Crear carpeta docs/ con subcarpetas organizadas
- Mover OTA_USAGE.md y MIGRATION_GUIDE.md a docs/guides/
- Agregar docs/README.md como índice principal
- Agregar QUICK_START.md para inicio rápido"

echo "✅ Commit 2/3 completado"

# Commit 3
git add src/config/Config.h src/config/Config_local.h.example
git commit -m "feat: Implementar sistema de configuración local

- Actualizar Config.h para soportar Config_local.h
- Agregar Config_local.h.example como template
- Permitir sobrescritura segura de credenciales
- Mejorar seguridad evitando hardcodear passwords"

echo "✅ Commit 3/3 completado"
echo ""
echo "📊 Resumen de commits:"
git log --oneline -3
echo ""
echo "✨ ¡Listo! Ahora ejecuta:"
echo "   git push origin main"