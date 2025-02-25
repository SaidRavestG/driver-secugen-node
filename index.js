const express = require('express');
const cors = require('cors');
const sg = require('./lib/sg');
const db = require('./lib/db'); // conexión a tu BD (ajústala tú)
const { execFile } = require("child_process");
const fs = require('fs');

const app = express();
app.use(cors());
app.use(express.json({limit:'2mb'}));
const TEMPLATE_PATH = "./temp_fingerprint.bin"; // Archivo temporal para la huella capturada
const MATCH_EXECUTABLE = "./match_template"; // Ejecutable para la comparación
app.get('/fingerprint/template', async (req, res) => {
  try {
    const template = await sg.getFingerprintTemplate();
    res.json({success:true, template});
  } catch (error) {
    res.status(500).json({success:false, error});
  }
});


app.post('/fingerprint/save', async (req, res) => {
  const { userId, fingername } = req.body;

  if (!userId || !fingername) {
      return res.status(400).json({ success: false, error: 'userId y fingername son requeridos' });
  }

  try {
      // Capturar la huella en formato binario
      const templateBuffer = await sg.getFingerprintTemplate();  
      console.log(`📥 Huella capturada para usuario ${userId}, tamaño: ${templateBuffer.length} bytes`);

      if (templateBuffer.length !== 378) {
          return res.status(400).json({ success: false, error: `El tamaño de la huella es incorrecto: ${templateBuffer.length} bytes` });
      }

      // Guardar en la base de datos correctamente como BLOB
      await db.query(
          'INSERT INTO fingerprints (user_id, fingername, fingerprint) VALUES (?, ?, ?)',
          [userId, fingername, templateBuffer]
      );

      res.json({ success: true, message: 'Huella guardada correctamente' });
  } catch (error) {
      console.error("❌ Error guardando huella:", error);
      res.status(500).json({ success: false, error });
  }
});

app.post("/fingerprint/verify", async (req, res) => {
  try {
      console.log("🔍 Capturando huella...");
      await new Promise((resolve, reject) => {
          execFile("./set_led", ["template"], { encoding: "buffer" }, (error, stdout, stderr) => {
              if (error) {
                  return reject(`Error capturando huella: ${stderr.toString()}`);
              }
              fs.writeFileSync("temp_fingerprint.bin", stdout); // Guardar template capturado
              console.log(`📂 Huella capturada, tamaño: ${stdout.length} bytes`);
              resolve();
          });
      });

      console.log("📂 Consultando base de datos...");
      const [rows] = await db.execute("SELECT user_id, fingerprint FROM fingerprints");

      for (const row of rows) {
          const userId = row.user_id;
          const fingerprintBuffer = Buffer.from(dbResult[0].fingerprint, 'binary');
          const fingerprintTrimmed = fingerprintBuffer.slice(-378); // Asegurar que solo queden 378 bytes
          fs.writeFileSync("db_fingerprint.bin", fingerprintTrimmed);

          // Verificar tamaño antes de continuar
          if (fingerprintBuffer.length !== 378) {
              console.log(`❌ Tamaño incorrecto en BD para usuario ${userId}: ${fingerprintBuffer.length} bytes`);
              continue;  // Saltar esta huella y seguir con la siguiente
          }

          // Guardar la huella recuperada en un archivo
          fs.writeFileSync("db_fingerprint.bin", fingerprintBuffer);
          console.log(`⚖️ Comparando con huella de usuario ID ${userId}, tamaño: ${fingerprintBuffer.length} bytes`);

          // Ejecutar la comparación
          const matchResult = await new Promise((resolve, reject) => {
              execFile("./match_template", ["temp_fingerprint.bin", "db_fingerprint.bin"], (error, stdout, stderr) => {
                  if (error) {
                      console.log(`❌ Error en comparación con usuario ID ${userId}: ${stderr.toString()}`);
                      return resolve(false);
                  }
                  resolve(stdout.includes("✅ Las huellas coinciden"));
              });
          });

          if (matchResult) {
              console.log(`✅ Huella coincidente encontrada: Usuario ID ${userId}`);
              return res.json({ success: true, userId });
          }
      }

      console.log("❌ No se encontraron coincidencias en la base de datos.");
      res.json({ success: false, message: "No se encontraron coincidencias" });
  } catch (error) {
      console.error("❌ Error en verificación:", error);
      res.status(500).json({ success: false, error: error.toString() });
  }
});
app.listen(3000, () => console.log('API lista en puerto 3000'));
