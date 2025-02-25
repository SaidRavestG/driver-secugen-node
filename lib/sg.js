const fs = require('fs');
const { execFile } = require('child_process');

module.exports = {
    // Captura la huella digital desde el sensor y devuelve el template
    getFingerprintTemplate: () => {
        return new Promise((resolve, reject) => {
            execFile("./set_led", ["template"], { encoding: "buffer" }, (error, stdout, stderr) => {
                if (error) {
                    return reject(`❌ Error capturando huella: ${stderr.toString()}`);
                }

                console.log(`📥 Huella capturada, tamaño: ${stdout.length} bytes`);

                // Verificar si tiene un encabezado extra (504 bytes en vez de 378)
                if (stdout.length === 504) {
                    console.log("📌 Eliminando encabezado de 126 bytes...");
                    stdout = stdout.slice(126); // Quitar los primeros 126 bytes
                }

                // Validar que el tamaño sea el correcto
                if (stdout.length !== 378) {
                    return reject(`❌ El tamaño de la huella es incorrecto después de la limpieza: ${stdout.length} bytes`);
                }

                resolve(stdout); // Devuelve el template binario
            });
        });
    },

    // Compara dos templates de huella digital
    matchTemplates: (base64Tpl1, base64Tpl2) => {
        return new Promise((resolve, reject) => {
            const tpl1 = Buffer.from(base64Tpl1, 'base64');
            const tpl2 = Buffer.from(base64Tpl2, 'base64');

            if (tpl1.length !== 378 || tpl2.length !== 378) {
                return reject({ message: "❌ Error: Templates no tienen 378 bytes" });
            }

            fs.writeFileSync('tpl1.bin', tpl1);
            fs.writeFileSync('tpl2.bin', tpl2);

            execFile("./match_template", ["tpl1.bin", "tpl2.bin"], (error, stdout, stderr) => {
                fs.unlinkSync('tpl1.bin');
                fs.unlinkSync('tpl2.bin');

                if (error) {
                    console.error("🔴 Error en match_template:", stderr.toString());
                    return reject(stderr.toString());
                }

                if (stdout.includes("✅ Las huellas coinciden")) {
                    resolve(true);
                } else {
                    resolve(false);
                }
            });
        });
    }
};
