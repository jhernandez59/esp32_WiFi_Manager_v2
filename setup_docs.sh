#!/bin/bash

# Script para crear la estructura de documentación
# Uso: ./setup_docs.sh

echo "╔════════════════════════════════════════════╗"
echo "║  Creando Estructura de Documentación       ║"
echo "╚════════════════════════════════════════════╝"
echo ""

# Crear directorios principales
echo "📁 Creando directorios..."
mkdir -p docs/{getting-started,guides,api,architecture,examples}

# Verificar creación
if [ -d "docs" ]; then
    echo "✅ Directorio 'docs' creado"
else
    echo "❌ Error: No se pudo crear 'docs'"
    exit 1
fi

# Crear archivos README vacíos como placeholders
echo ""
echo "📝 Creando archivos placeholder..."

# README principal de docs
cat > docs/README.md << 'EOF'
# 📚 Documentación

Ver archivos en las subcarpetas para documentación detallada.

## Índice

- [Getting Started](getting-started/) - Para empezar
- [Guides](guides/) - Guías específicas
- [API Reference](api/) - Referencia técnica
- [Architecture](architecture/) - Arquitectura del proyecto
- [Examples](examples/) - Ejemplos prácticos
EOF

# Getting Started
touch docs/getting-started/INSTALLATION.md
touch docs/getting-started/FIRST_STEPS.md

# Guides - estos ya los moveremos
touch docs/guides/WIFI_SETUP.md
touch docs/guides/TROUBLESHOOTING.md

# API Reference
touch docs/api/MODULES_API.md
touch docs/api/WIFIMANAGER_API.md
touch docs/api/FILEMANAGER_API.md
touch docs/api/LEDCONTROLLER_API.md
touch docs/api/WEBSERVER_API.md
touch docs/api/OTAMANAGER_API.md

# Architecture
touch docs/architecture/DESIGN_PATTERNS.md
touch docs/architecture/MODULE_STRUCTURE.md
touch docs/architecture/CODING_STANDARDS.md

# Examples
touch docs/examples/BASIC_USAGE.md
touch docs/examples/ADVANCED_USAGE.md
touch docs/examples/CUSTOM_MODULES.md

echo "✅ Archivos placeholder creados"

# Instrucciones para mover archivos existentes
echo ""
echo "╔════════════════════════════════════════════╗"
echo "║  Próximos Pasos                           ║"
echo "╚════════════════════════════════════════════╝"
echo ""
echo "📋 Ahora debes:"
echo ""
echo "1. Mover archivos existentes:"
echo "   mv OTA_USAGE.md docs/guides/"
echo "   mv MIGRATION_GUIDE.md docs/guides/"
echo ""
echo "2. Copiar contenido de artifacts a:"
echo "   - docs/README.md"
echo "   - docs/getting-started/QUICK_START.md"
echo "   - docs/guides/OTA_USAGE.md"
echo "   - docs/guides/MIGRATION_GUIDE.md"
echo ""
echo "3. Actualizar README.md principal con enlaces a docs/"
echo ""
echo "✨ Estructura creada exitosamente!"
echo ""

# Mostrar árbol de directorios
echo "📁 Estructura creada:"
echo ""
tree -L 2 docs/ 2>/dev/null || find docs/ -type f -o -type d | sed 's|[^/]*/| |g'

echo ""
echo "✅ ¡Listo! La estructura de documentación está creada."