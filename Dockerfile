# Usar Ubuntu como base en lugar de la imagen node genérica
FROM ubuntu:22.04

# Configurar timezone para evitar interacciones durante la instalación
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Mexico_City

# Instalar Node.js 16 (en lugar de 18) y herramientas de compilación
RUN apt-get update && apt-get install -y \
    curl \
    build-essential \
    gcc \
    g++ \
    make \
    libusb-1.0-0-dev \
    udev \
    python3 \
    python-is-python3 \
    && curl -fsSL https://deb.nodesource.com/setup_16.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio de trabajo
WORKDIR /app

# Copiar archivos del proyecto
COPY package*.json ./
COPY index.js ./
COPY match_template.c ./
COPY set_led.c ./
COPY lib/ ./lib/

# Copiar todas las bibliotecas SecuGen necesarias
COPY secugen/lib/libsgfplib.so /usr/local/lib/
COPY secugen/lib/libsgimage.so* /usr/local/lib/
COPY secugen/lib/libsgfpamx.so* /usr/local/lib/
COPY secugen/lib/libsgnfiq.so* /usr/local/lib/
COPY secugen/lib/libAKXUS.so* /usr/local/lib/
COPY secugen/lib/libuvc.so* /usr/local/lib/
COPY secugen/include/sgfplib.h ./

# Crear enlaces simbólicos para las bibliotecas con versiones
RUN ln -sf /usr/local/lib/libsgfpamx.so.3.7.0 /usr/local/lib/libsgfpamx.so.3
RUN ln -sf /usr/local/lib/libsgfpamx.so.3 /usr/local/lib/libsgfpamx.so
RUN ln -sf /usr/local/lib/libsgimage.so.1.0.0 /usr/local/lib/libsgimage.so.1
RUN ln -sf /usr/local/lib/libsgimage.so.1 /usr/local/lib/libsgimage.so
RUN ln -sf /usr/local/lib/libsgnfiq.so.1.0.0 /usr/local/lib/libsgnfiq.so.1
RUN ln -sf /usr/local/lib/libsgnfiq.so.1 /usr/local/lib/libsgnfiq.so
RUN ln -sf /usr/local/lib/libAKXUS.so.2.0.11 /usr/local/lib/libAKXUS.so.2
RUN ln -sf /usr/local/lib/libAKXUS.so.2 /usr/local/lib/libAKXUS.so
RUN ln -sf /usr/local/lib/libuvc.so.0.0.6 /usr/local/lib/libuvc.so.0
RUN ln -sf /usr/local/lib/libuvc.so.0 /usr/local/lib/libuvc.so

# Instalar dependencias de Node.js
RUN npm install

# Configurar reglas udev para el lector SecuGen
RUN mkdir -p /etc/udev/rules.d
RUN if [ -f secugen/99-secugen.rules ]; then \
    cp secugen/99-secugen.rules /etc/udev/rules.d/; \
    else echo "No rules file found, will use default USB permissions"; \
    fi

# Dar permisos de ejecución a todas las bibliotecas
RUN chmod 755 /usr/local/lib/libsgfplib.so
RUN chmod 755 /usr/local/lib/libsgimage.so*
RUN chmod 755 /usr/local/lib/libsgfpamx.so*
RUN chmod 755 /usr/local/lib/libsgnfiq.so*
RUN chmod 755 /usr/local/lib/libAKXUS.so*
RUN chmod 755 /usr/local/lib/libuvc.so*

# Actualizar cache de bibliotecas
RUN ldconfig

# Compilar los archivos C con g++ (ya que la biblioteca usa C++)
RUN g++ match_template.c -o match_template -L/usr/local/lib -lsgfplib -lstdc++
RUN g++ set_led.c -o set_led -L/usr/local/lib -lsgfplib -lstdc++

# Dar permisos de ejecución a los binarios compilados
RUN chmod +x match_template set_led

# Exponer el puerto
EXPOSE 3000

# Comando para iniciar la aplicación
CMD ["node", "index.js"] 