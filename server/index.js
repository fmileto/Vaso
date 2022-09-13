const express = require('express');
const app = express();
const port = 7450;

app.get('/vaso/sync/plant/:id', (req, res) => {
  console.log(`GET ${req.url}`);
  res.json({ soilMoistureCritical: 30 });
});

app.get('/vaso/measures', (req, res) => {
  console.log(`GET ${req.url}`);
  res.json({ ok: true });
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`);
});
