/*
Operaciones con LittleFS:

Leer archivos
Escribir archivos
Eliminar archivos
Verificar existencia
*/
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>

class FileManager {
private:
    static FileManager* instance;
    FileManager(); // Constructor privado para Singleton
    
public:
    /**
     * Obtiene la instancia única de FileManager (Singleton)
     * @return Puntero a la instancia de FileManager
     */
    static FileManager* getInstance();
    
    /**
     * Inicializa el sistema de archivos LittleFS
     * @return true si se inicializó correctamente
     */
    bool begin();
    
    /**
     * Lee el contenido completo de un archivo
     * @param path Ruta del archivo
     * @return String con el contenido (vacío si hay error)
     */
    String readFile(const char* path);
    
    /**
     * Escribe contenido en un archivo (sobrescribe si existe)
     * @param path Ruta del archivo
     * @param content Contenido a escribir
     * @return true si se escribió correctamente
     */
    bool writeFile(const char* path, const String& content);
    
    /**
     * Añade contenido al final de un archivo
     * @param path Ruta del archivo
     * @param content Contenido a añadir
     * @return true si se añadió correctamente
     */
    bool appendFile(const char* path, const String& content);
    
    /**
     * Verifica si un archivo existe
     * @param path Ruta del archivo
     * @return true si el archivo existe
     */
    bool exists(const char* path);
    
    /**
     * Elimina un archivo
     * @param path Ruta del archivo
     * @return true si se eliminó correctamente
     */
    bool deleteFile(const char* path);
    
    /**
     * Elimina todos los archivos de configuración WiFi
     * @return true si se eliminaron correctamente
     */
    bool clearWiFiConfig();
    
    /**
     * Obtiene el tamaño de un archivo
     * @param path Ruta del archivo
     * @return Tamaño en bytes (0 si hay error)
     */
    size_t getFileSize(const char* path);
    
    /**
     * Lista todos los archivos en el sistema de archivos
     */
    void listFiles();
};

#endif // FILEMANAGER_H