const db = require('./db');

async function testConnection() {
  try {
    const result = await db.query('SELECT NOW() AS time');
    console.log('✅ Conexión exitosa. Hora actual:', result[0].time);
  } catch (error) {
    console.error('❌ Error conectando a la base de datos:', error);
  } finally {
    process.exit();
  }
}

testConnection();