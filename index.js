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
              fs.writeFileSync("temp_fingerprint.bin", stdout);
              console.log(`📂 Huella capturada, tamaño: ${stdout.length} bytes`);
              resolve();
          });
      });

      console.log("📂 Consultando base de datos...");
      const [rows] = await db.execute("SELECT user_id, fingerprint FROM fingerprints");

      for (const row of rows) {
          const userId = row.user_id;
          const fingerprintBuffer = Buffer.from(row.fingerprint, 'binary');
          const fingerprintTrimmed = fingerprintBuffer.slice(-378);
          fs.writeFileSync("db_fingerprint.bin", fingerprintTrimmed);

          // Verificar tamaño antes de continuar
          if (fingerprintTrimmed.length !== 378) {
              console.log(`❌ Tamaño incorrecto en BD para usuario ${userId}: ${fingerprintTrimmed.length} bytes`);
              continue;
          }

          console.log(`⚖️ Comparando con huella de usuario ID ${userId}, tamaño: ${fingerprintTrimmed.length} bytes`);

          const matchResult = await new Promise((resolve, reject) => {
              execFile("./match_template", ["temp_fingerprint.bin", "db_fingerprint.bin"], 
              { encoding: 'utf8' },  // Agregamos encoding explícito
              (error, stdout, stderr) => {
                  // Siempre mostrar la salida completa
                  console.log(`\n📊 Resultado detallado para usuario ${userId}:`);
                  if (stdout) console.log("Stdout:", stdout);
                  if (stderr) console.log("Stderr:", stderr);
                  
                  if (error) {
                      console.log(`❌ Error ejecutando match_template:`, error);
                      return resolve(false);
                  }

                  // Verificar si hay coincidencia y mostrar el puntaje
                  const matched = stdout.includes("✅ Las huellas coinciden");
                  const scoreMatch = stdout.match(/Puntaje de coincidencia: (\d+)/);
                  const score = scoreMatch ? scoreMatch[1] : 'N/A';
                  
                  console.log(`📊 Puntaje: ${score}`);
                  console.log(`${matched ? '👍' : '👎'} Coincidencia: ${matched}`);
                  
                  resolve(matched);
              });
          });

          if (matchResult) {
              // Verificar archivos temporales
              console.log("\n📂 Verificando archivos temporales:");
              const tempContent = fs.readFileSync("temp_fingerprint.bin");
              const dbContent = fs.readFileSync("db_fingerprint.bin");
              console.log(`temp_fingerprint.bin: ${tempContent.length} bytes, primeros 10: ${tempContent.slice(0,10).toString('hex')}`);
              console.log(`db_fingerprint.bin: ${dbContent.length} bytes, primeros 10: ${dbContent.slice(0,10).toString('hex')}`);
              
              console.log(`✅ Huella coincidente encontrada: Usuario ID ${userId}`);
              return res.json({ success: true, userId });
          }
      }

      console.log("❌ No se encontraron coincidencias en la base de datos.");
      res.json({ success: false, message: "No se encontraron coincidencias" });
  } catch (error) {
      console.error("❌ Error verificando huellas:", error);
      res.status(500).json({ success: false, error });
  } finally {
      // Limpieza de archivos temporales
      try {
          if (fs.existsSync("temp_fingerprint.bin")) fs.unlinkSync("temp_fingerprint.bin");
          if (fs.existsSync("db_fingerprint.bin")) fs.unlinkSync("db_fingerprint.bin");
      } catch (error) {
          console.error("Error limpiando archivos temporales:", error);
      }
  }
});

app.listen(3000, () => {
  console.log('Servidor corriendo en http://localhost:3000');
});
