#!/usr/bin/env python3
"""Launch the self-contained installer locally; Python standard library only."""
import functools,http.server,re,threading,webbrowser
from pathlib import Path
ROOT=Path(__file__).resolve().parent
if not (ROOT/'index.html').exists():ROOT=ROOT.parent/'dist/install'
if not (ROOT/'manifest.json').is_file():raise SystemExit('Usa il pacchetto installer compilato oppure esegui npm run installer:build.')
class Handler(http.server.SimpleHTTPRequestHandler):
 extensions_map={**http.server.SimpleHTTPRequestHandler.extensions_map,'.js':'text/javascript','.css':'text/css','.bin':'application/octet-stream','.json':'application/json','.zip':'application/zip'}
 def do_GET(self):
  if self.headers.get('Host') not in (f'127.0.0.1:{self.server.server_port}',f'localhost:{self.server.server_port}'):
   self.send_error(403);return
  if not re.fullmatch(r'/(?:index\.html|personalizzazione\.html|manifest\.json|downloads/PocketDD-PC\.zip|assets/[\w.-]+\.(?:js|css|png)|firmware/(?:bootloader|partitions|app)\.bin|licenses/[\w.-]+\.txt)?',self.path):
   self.send_error(404);return
  super().do_GET()
 def do_HEAD(self):self.send_error(405)
 def end_headers(self):
  self.send_header('Cache-Control','no-store');self.send_header('X-Content-Type-Options','nosniff');self.send_header('Referrer-Policy','no-referrer');super().end_headers()
try:server=http.server.ThreadingHTTPServer(('127.0.0.1',8787),functools.partial(Handler,directory=str(ROOT)))
except OSError:raise SystemExit('Porta 8787 occupata: chiudi l’altro installer e riprova.')
url='http://127.0.0.1:8787/'
print(f'PocketD-D installer: {url}\nApri in Chrome o Edge. Lascia questa finestra aperta; Ctrl+C per chiudere.',flush=True)
threading.Timer(.5,lambda:webbrowser.open(url)).start()
try:server.serve_forever()
except KeyboardInterrupt:pass
finally:server.server_close()
