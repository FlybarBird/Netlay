module.exports = (function () {
  'use strict';

  var BG = '#1B1D23';
  var SIDEBAR = '#16181E';
  var HEADER = '#1A1B1E';
  var CARD = '#202124';
  var RAISED = '#252729';
  var WELL = '#14161B';
  var ACCENT = '#3581D6';
  var TEXT = '#F2F3F5';
  var MUTED = '#9CA3AF';
  var LINE = 'rgba(255,255,255,0.2)';
  var DANGER = '#844B43';
  var OK = '#26BA5A';
  var METER = 'linear-gradient(90deg,#1A5FBF 0%,#2FB0EE 62%,#47C0E9 100%)';
  var FONT = '"SF Pro Display", "SF Pro Text", -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif';

  function inputStyle() {
    return {
      width: '100%',
      boxSizing: 'border-box',
      background: WELL,
      color: TEXT,
      border: '1px solid ' + LINE,
      borderRadius: '8px',
      padding: '10px 12px',
      fontSize: '14px',
      outline: 'none',
      fontFamily: FONT
    };
  }

  function label(h, text) {
    return h('div', {
      style: {
        color: MUTED,
        fontSize: '11px',
        letterSpacing: '0.08em',
        textTransform: 'uppercase',
        marginBottom: '6px',
        fontFamily: FONT
      }
    }, text);
  }

  function field(h, lab, node) {
    return h('div', { style: { marginBottom: '16px' } }, [label(h, lab), node]);
  }

  function cardStyle() {
    return {
      background: CARD,
      borderRadius: '10px',
      padding: '22px',
      border: '1px solid ' + LINE
    };
  }

  return {
    name: 'NetlayView',

    data: function () {
      return {
        loading: true,
        busy: false,
        supportBusy: false,
        error: null,
        page: 'network',
        state: 'stopped',
        server: 'aoo.sonobus.net',
        port: 10998,
        group: '',
        password: '',
        username: 'Slate7',
        device: 'auto',
        volume: 0.8,
        allowRemoteMix: true,
        controller: '',
        devices: [],
        peers: [],
        alsaOk: false,
        audioHint: '',
        support: null,
        encrypted: false,
        peakL: 0,
        peakR: 0,
        rmsL: 0,
        rmsR: 0,
        clip: false,
        timer: null,
        meterTimer: null
      };
    },

    created: function () {
      this.refresh();
      this.refreshSupport();
      this.timer = setInterval(this.refresh, 3000);
      this.meterTimer = setInterval(this.refreshMeter, 120);
    },

    beforeDestroy: function () {
      if (this.timer) clearInterval(this.timer);
      if (this.meterTimer) clearInterval(this.meterTimer);
    },

    computed: {
      connected: function () {
        return this.state === 'in_group' || this.state === 'receiving' || this.state === 'connected';
      },
      stateLabel: function () {
        var map = {
          stopped: 'Disconnected',
          idle: 'Disconnected',
          connecting: 'Connecting…',
          connected: 'Connected',
          in_group: 'Connected',
          receiving: 'Receiving audio',
          error: 'Error'
        };
        return map[this.state] || this.state;
      }
    },

    methods: {
      rpc: function (fn, args) {
        var clean = {};
        var src = args || {};
        Object.keys(src).forEach(function (k) {
          var v = src[k];
          if (v === undefined || v === null || v === '') return;
          clean[k] = typeof v === 'string' ? v : String(v);
        });
        if (typeof clean.device === 'string') {
          clean.device = clean.device.replace(/,/g, '.');
        }
        if (typeof window.$rpcRequest === 'function') {
          return window.$rpcRequest('call', ['sid', 'netlay', fn, clean]);
        }
        var token = (document.cookie.match(/Admin-Token=([^;]+)/) || [])[1] || '';
        return fetch('/rpc', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            jsonrpc: '2.0',
            id: Date.now(),
            method: 'call',
            params: [token, 'netlay', fn, clean]
          })
        }).then(function (r) { return r.json(); }).then(function (d) {
          if (d.error) throw new Error(d.error.message || 'RPC error');
          return d.result || {};
        });
      },

      applyPayload: function (data) {
        if (!data) return;
        this.state = data.state || this.state;
        this.error = data.error || null;
        this.peers = data.peers || [];
        this.alsaOk = !!data.alsa_ok;
        this.audioHint = (data.audio && data.audio.hint) || '';
        if (typeof data.volume === 'number') this.volume = data.volume;
        if (typeof data.allow_remote_mix === 'boolean') this.allowRemoteMix = data.allow_remote_mix;
        if (typeof data.controller === 'string') this.controller = data.controller;
        this.applyLevels(data);
        if (data.devices && data.devices.length) this.devices = data.devices;
        var u = data.uci || {};
        if (u.server) this.server = u.server;
        if (u.port) this.port = Number(u.port);
        if (u.group != null && !this.connected) this.group = u.group;
        if (u.password != null && !this.connected) this.password = u.password;
        if (u.username) this.username = u.username;
        if (u.allow_remote_mix != null && !this.connected) {
          this.allowRemoteMix = u.allow_remote_mix === '1' || u.allow_remote_mix === true || u.allow_remote_mix === 'true';
        }
        var picked = this.normalizeDevice(this.device);
        if (!this.connected) {
          if (u.device) picked = this.normalizeDevice(u.device);
        }
        if (data.device) {
          var fromDaemon = this.normalizeDevice(data.device);
          if (fromDaemon === 'auto' || this.deviceInList(fromDaemon)) picked = fromDaemon;
        }
        if (picked === 'auto' || this.deviceInList(picked)) this.device = picked;
        else if (!this.deviceInList(this.device)) this.device = 'auto';
        if (data.group && this.connected) this.group = data.group;
        if (data.username && this.connected) this.username = data.username;
      },

      normalizeDevice: function (id) {
        if (!id || id === 'default') return 'auto';
        id = String(id).replace(/^plug/, '');
        return id.replace(/(\d)\.(\d)/, '$1,$2');
      },

      deviceInList: function (id) {
        var want = this.normalizeDevice(id);
        return (this.devices || []).some(function (d) {
          return d && d.id === want;
        });
      },

      applyLevels: function (data) {
        if (!data) return;
        var lv = data.levels || data;
        if (typeof lv.peak_l === 'number') this.peakL = lv.peak_l;
        if (typeof lv.peak_r === 'number') this.peakR = lv.peak_r;
        if (typeof lv.rms_l === 'number') this.rmsL = lv.rms_l;
        if (typeof lv.rms_r === 'number') this.rmsR = lv.rms_r;
        if (typeof lv.clip === 'boolean') this.clip = lv.clip;
      },

      refreshMeter: function () {
        var self = this;
        this.rpc('levels', {}).then(function (data) {
          self.applyLevels(data);
        }).catch(function () {});
      },

      linToDb: function (x) {
        if (!(x > 0.0001)) return -60;
        return Math.max(-60, 20 * Math.log10(x));
      },

      dbToPct: function (db) {
        return Math.max(0, Math.min(1, (db + 60) / 60));
      },

      formatDb: function (x) {
        var db = this.linToDb(x);
        if (db <= -59.5) return '-∞';
        return (db >= 0 ? '+' : '') + db.toFixed(1);
      },

      meterBar: function (h, name, peak, rms) {
        var peakPct = this.dbToPct(this.linToDb(peak));
        var rmsPct = this.dbToPct(this.linToDb(rms));
        var clip = this.clip || peak >= 0.99;
        return h('div', { style: { display: 'flex', alignItems: 'center', gap: '8px', marginBottom: '8px' } }, [
          h('span', { style: { width: '14px', fontSize: '11px', color: MUTED, fontWeight: '700' } }, name),
          h('div', {
            style: {
              position: 'relative',
              flex: '1',
              height: '14px',
              borderRadius: '3px',
              background: WELL,
              border: '1px solid ' + LINE,
              overflow: 'hidden'
            }
          }, [
            h('div', { style: { position: 'absolute', inset: '0', background: METER, opacity: '0.22' } }),
            h('div', {
              style: {
                position: 'absolute', left: '0', top: '0', bottom: '0',
                width: (rmsPct * 100).toFixed(1) + '%',
                background: METER, opacity: '0.7'
              }
            }),
            h('div', {
              style: {
                position: 'absolute',
                left: 'calc(' + (peakPct * 100).toFixed(1) + '% - 1px)',
                top: '0', bottom: '0', width: '2px',
                background: clip ? DANGER : TEXT
              }
            }),
            h('div', { style: { position: 'absolute', left: '70%', top: '0', bottom: '0', width: '1px', background: 'rgba(255,255,255,0.12)' } }),
            h('div', { style: { position: 'absolute', left: '85%', top: '0', bottom: '0', width: '1px', background: 'rgba(255,255,255,0.12)' } })
          ]),
          h('span', {
            style: {
              width: '44px', textAlign: 'right', fontSize: '11px',
              fontVariantNumeric: 'tabular-nums', color: clip ? DANGER : MUTED
            }
          }, this.formatDb(peak) + ' dB')
        ]);
      },

      refresh: function () {
        var self = this;
        this.rpc('status', {}).then(function (data) {
          self.loading = false;
          self.applyPayload(data);
        }).catch(function (e) {
          self.loading = false;
          self.error = String(e && e.message ? e.message : e);
        });
      },

      refreshSupport: function () {
        var self = this;
        this.rpc('support', {}).then(function (data) {
          self.support = data;
        }).catch(function () {});
      },

      installSupport: function () {
        var self = this;
        self.supportBusy = true;
        self.rpc('install_support', {}).then(function (data) {
          self.supportBusy = false;
          self.support = data;
          self.refresh();
        }).catch(function (e) {
          self.supportBusy = false;
          self.error = String(e && e.message ? e.message : e);
        });
      },

      connect: function () {
        var self = this;
        if (!self.group) {
          self.error = 'Enter a group name';
          return;
        }
        self.busy = true;
        self.rpc('connect', {
          server: self.server,
          port: self.port,
          group: self.group,
          password: self.password,
          username: self.username,
          device: self.device,
          volume: self.volume,
          allow_remote_mix: self.allowRemoteMix ? '1' : '0',
          enabled: '1'
        }).then(function (data) {
          self.busy = false;
          self.applyPayload(data);
        }).catch(function (e) {
          self.busy = false;
          self.error = String(e && e.message ? e.message : e);
        });
      },

      disconnect: function () {
        var self = this;
        self.busy = true;
        self.rpc('disconnect', {}).then(function (data) {
          self.busy = false;
          self.applyPayload(data);
        }).catch(function (e) {
          self.busy = false;
          self.error = String(e && e.message ? e.message : e);
        });
      },

      onVolume: function (e) {
        var v = Number(e.target.value);
        this.volume = v;
        this.rpc('volume', { value: String(v) }).then(this.applyPayload.bind(this)).catch(function () {});
      },

      onAllowRemote: function (e) {
        var on = !!e.target.checked;
        this.allowRemoteMix = on;
        this.rpc('allow_remote_mix', { value: on ? '1' : '0' }).then(this.applyPayload.bind(this)).catch(function () {});
      },

      setPage: function (page) {
        this.page = page;
      },

      navButton: function (h, id, title) {
        var self = this;
        var on = self.page === id;
        return h('button', {
          style: {
            display: 'block',
            width: '100%',
            height: '36px',
            marginBottom: '6px',
            border: 'none',
            borderRadius: '8px',
            background: on ? ACCENT : 'transparent',
            color: on ? TEXT : MUTED,
            fontSize: '14px',
            fontWeight: on ? '700' : '500',
            letterSpacing: on ? '0' : '0.04em',
            textAlign: 'left',
            padding: '0 14px',
            cursor: 'pointer',
            fontFamily: FONT
          },
          on: { click: function () { self.setPage(id); } }
        }, title);
      },

      renderNetwork: function (h) {
        var self = this;
        return h('div', { style: { maxWidth: '560px' } }, [
          self.error ? h('div', { style: { color: '#e88', fontSize: '13px', marginBottom: '14px' } }, self.error) : null,
          h('div', { style: cardStyle() }, [
            h('div', {
              style: { fontSize: '15px', fontWeight: '700', color: TEXT, marginBottom: '18px', letterSpacing: '0.02em' }
            }, 'Private group'),
            field(h, 'Group name', h('input', {
              style: inputStyle(),
              attrs: { type: 'text', placeholder: 'unique-group-name', value: self.group },
              on: { input: function (e) { self.group = e.target.value; } }
            })),
            field(h, 'Group password', h('input', {
              style: inputStyle(),
              attrs: { type: 'password', placeholder: 'optional', value: self.password },
              on: { input: function (e) { self.password = e.target.value; } }
            })),
            field(h, 'Your display name', h('input', {
              style: inputStyle(),
              attrs: { type: 'text', value: self.username },
              on: { input: function (e) { self.username = e.target.value; } }
            })),
            h('button', {
              style: {
                width: '100%',
                padding: '12px',
                border: 'none',
                borderRadius: '8px',
                background: self.connected ? RAISED : ACCENT,
                color: TEXT,
                fontSize: '15px',
                fontWeight: '700',
                cursor: self.busy ? 'wait' : 'pointer',
                fontFamily: FONT
              },
              attrs: { disabled: self.busy },
              on: { click: self.connected ? self.disconnect : self.connect }
            }, self.busy ? 'Working…' : (self.connected ? 'Disconnect' : 'Connect'))
          ]),
          h('div', {
            style: Object.assign({}, cardStyle(), { marginTop: '16px', padding: '16px 22px' })
          }, [
            h('div', { style: { fontSize: '13px', fontWeight: '700', color: TEXT } }, 'USB playback'),
            h('div', {
              style: { fontSize: '12px', color: MUTED, marginTop: '4px', letterSpacing: '0.04em' }
            }, self.alsaOk ? 'Output is open' : (self.audioHint || 'Set the device under Settings, then Connect'))
          ])
        ]);
      },

      renderPeers: function (h) {
        var self = this;
        var colors = ['#3581D6', '#2FB0EE', '#3DDC97', '#E0B43A', '#E07A3A', '#E05B7A', '#9B7AE0', '#5BB8A4'];
        var rows = (self.peers || []).length
          ? self.peers.map(function (p, i) {
            return h('div', {
              style: {
                display: 'flex',
                alignItems: 'center',
                gap: '12px',
                padding: '12px 0',
                borderBottom: '1px solid ' + LINE
              }
            }, [
              h('div', {
                style: {
                  width: '8px',
                  height: '28px',
                  borderRadius: '4px',
                  background: colors[i % colors.length],
                  flexShrink: '0'
                }
              }),
              h('div', { style: { flex: '1', minWidth: '0' } }, [
                h('div', { style: { color: TEXT, fontSize: '14px', fontWeight: '700' } }, p.user || 'peer'),
                h('div', { style: { color: MUTED, fontSize: '12px', letterSpacing: '0.04em' } }, p.address || '')
              ]),
              h('div', { style: { color: MUTED, fontSize: '12px' } }, p.mute ? 'Muted' : '')
            ]);
          })
          : [h('div', { style: { color: MUTED, fontSize: '13px', letterSpacing: '0.04em' } },
            self.connected ? 'No peers yet' : 'Connect to a group to see peers')];

        return h('div', { style: { maxWidth: '720px' } }, [
          h('div', { style: cardStyle() }, [
            h('div', {
              style: { fontSize: '15px', fontWeight: '700', color: TEXT, marginBottom: '12px' }
            }, 'Peers (' + ((self.peers && self.peers.length) || 0) + ')'),
            rows
          ]),
          h('div', { style: Object.assign({}, cardStyle(), { marginTop: '16px' }) }, [
            label(h, self.clip ? 'Output  CLIP' : 'Output'),
            self.meterBar(h, 'L', self.peakL, self.rmsL),
            self.meterBar(h, 'R', self.peakR, self.rmsR),
            h('div', { style: { marginTop: '14px' } }, [
              label(h, 'Volume  ' + Math.round(self.volume * 100) + '%'),
              h('input', {
                style: { width: '100%' },
                attrs: { type: 'range', min: '0', max: '1.5', step: '0.05', value: String(self.volume) },
                on: { input: self.onVolume }
              })
            ])
          ])
        ]);
      },

      renderSettings: function (h) {
        var self = this;
        var deviceOptions = [h('option', { attrs: { value: 'auto' } }, 'Auto')].concat(
          (self.devices || []).map(function (d) {
            return h('option', { attrs: { value: d.id } }, d.name || 'USB audio');
          })
        );
        var pkgs = (self.support && self.support.packages) || [];
        var supportRows = pkgs.length
          ? pkgs.map(function (p) {
            return h('div', {
              style: { display: 'flex', justifyContent: 'space-between', fontSize: '13px', padding: '6px 0', color: TEXT }
            }, [
              h('span', {}, p.name),
              h('span', { style: { color: p.installed ? OK : MUTED, letterSpacing: '0.04em' } }, p.installed ? 'installed' : 'missing')
            ]);
          })
          : [h('div', { style: { color: MUTED, fontSize: '12px' } }, 'Package status unknown')];

        return h('div', { style: { maxWidth: '560px' } }, [
          h('div', { style: cardStyle() }, [
            h('div', { style: { fontSize: '15px', fontWeight: '700', color: TEXT, marginBottom: '18px' } }, 'Audio'),
            field(h, 'USB audio device', h('select', {
              style: inputStyle(),
              attrs: { value: self.device },
              domProps: { value: self.device },
              on: { change: function (e) { self.device = e.target.value; } }
            }, deviceOptions)),
            h('div', {
              style: { fontSize: '12px', color: self.alsaOk ? OK : MUTED, marginBottom: '16px', lineHeight: '1.45', letterSpacing: '0.04em' }
            }, self.alsaOk ? 'USB/ALSA output is open' : (self.audioHint || 'No USB sound card yet')),
            h('label', {
              style: { display: 'flex', alignItems: 'center', gap: '10px', fontSize: '14px', color: TEXT, cursor: 'pointer' }
            }, [
              h('input', {
                attrs: { type: 'checkbox', checked: self.allowRemoteMix },
                domProps: { checked: self.allowRemoteMix },
                on: { change: self.onAllowRemote }
              }),
              h('span', {}, 'Allow others to control my mix')
            ])
          ]),
          h('div', { style: Object.assign({}, cardStyle(), { marginTop: '16px' }) }, [
            h('div', { style: { fontSize: '15px', fontWeight: '700', color: TEXT, marginBottom: '18px' } }, 'Connection server'),
            field(h, 'Server', h('div', { style: { display: 'flex', gap: '8px' } }, [
              h('input', {
                style: Object.assign({}, inputStyle(), { flex: '1' }),
                attrs: { type: 'text', value: self.server },
                on: { input: function (e) { self.server = e.target.value; } }
              }),
              h('input', {
                style: Object.assign({}, inputStyle(), { width: '90px' }),
                attrs: { type: 'number', value: self.port },
                on: { input: function (e) { self.port = Number(e.target.value); } }
              })
            ])),
            h('div', {
              style: { fontSize: '12px', color: MUTED, letterSpacing: '0.04em', lineHeight: '1.45' }
            }, 'Discovery only. Audio is peer-to-peer and is not encrypted.')
          ]),
          h('div', { style: Object.assign({}, cardStyle(), { marginTop: '16px' }) }, [
            h('div', { style: { fontSize: '15px', fontWeight: '700', color: TEXT, marginBottom: '8px' } }, 'Audio packages'),
            supportRows,
            (self.support && self.support.hint) ? h('div', {
              style: { color: MUTED, fontSize: '12px', marginTop: '8px', lineHeight: '1.4' }
            }, self.support.hint) : null,
            h('button', {
              style: {
                marginTop: '14px',
                width: '100%',
                padding: '10px',
                border: 'none',
                borderRadius: '8px',
                background: ACCENT,
                color: TEXT,
                fontSize: '13px',
                fontWeight: '700',
                cursor: self.supportBusy ? 'wait' : 'pointer',
                fontFamily: FONT
              },
              attrs: { disabled: self.supportBusy },
              on: { click: self.installSupport }
            }, self.supportBusy ? 'Installing…' : 'Install audio packages')
          ])
        ]);
      }
    },

    render: function (h) {
      var self = this;
      if (self.loading) {
        return h('div', {
          style: {
            padding: '48px', textAlign: 'center', color: MUTED, background: BG,
            minHeight: '60vh', fontFamily: FONT, letterSpacing: '0.08em'
          }
        }, 'Loading Netlay…');
      }

      var sidebar = h('div', {
        style: {
          width: '216px',
          flexShrink: '0',
          background: SIDEBAR,
          borderRight: '1px solid ' + LINE,
          padding: '18px 16px',
          display: 'flex',
          flexDirection: 'column',
          boxSizing: 'border-box'
        }
      }, [
        h('img', {
          attrs: { src: '/netlay-wordmark.png', alt: 'Netlay' },
          style: {
            width: '100%',
            height: '58px',
            objectFit: 'contain',
            objectPosition: 'left center',
            mixBlendMode: 'screen',
            marginBottom: '16px'
          }
        }),
        self.navButton(h, 'network', 'Network'),
        self.navButton(h, 'peers', 'Peers'),
        self.navButton(h, 'settings', 'Settings'),
        h('div', { style: { flex: '1' } }),
        h('div', {
          style: { fontSize: '12px', color: MUTED, letterSpacing: '0.08em', marginBottom: '8px' }
        }, 'Private group'),
        h('div', {
          style: { fontSize: '12px', color: MUTED, letterSpacing: '0.08em' }
        }, 'Based on SonoBus')
      ]);

      var extra = self.controller
        ? (self.controller + ' is controlling this mix')
        : (self.alsaOk ? 'USB out' : '');

      var header = h('div', {
        style: {
          height: '56px',
          flexShrink: '0',
          background: HEADER,
          borderBottom: '1px solid ' + LINE,
          display: 'flex',
          alignItems: 'center',
          padding: '0 16px',
          gap: '12px',
          boxSizing: 'border-box'
        }
      }, [
        h('div', {
          style: {
            minWidth: '118px',
            padding: '6px 14px',
            borderRadius: '8px',
            background: self.connected ? ACCENT : RAISED,
            color: self.connected ? TEXT : MUTED,
            fontSize: '13px',
            fontWeight: '700',
            textAlign: 'center'
          }
        }, self.stateLabel),
        h('div', { style: { flex: '1', minWidth: '0' } }, [
          h('div', {
            style: { fontSize: '16px', fontWeight: '700', color: TEXT, lineHeight: '1.2' }
          }, self.connected && self.group ? self.group : 'Not in a group'),
          h('div', {
            style: { fontSize: '13px', color: MUTED, letterSpacing: '0.06em' }
          }, self.username || '')
        ]),
        extra ? h('div', {
          style: { fontSize: '13px', color: MUTED, letterSpacing: '0.04em', textAlign: 'right' }
        }, extra) : null
      ]);

      var page = self.page === 'peers'
        ? self.renderPeers(h)
        : (self.page === 'settings' ? self.renderSettings(h) : self.renderNetwork(h));

      return h('div', {
        style: {
          display: 'flex',
          minHeight: 'calc(100vh - 96px)',
          margin: '-16px',
          background: BG,
          color: TEXT,
          fontFamily: FONT
        }
      }, [
        sidebar,
        h('div', { style: { flex: '1', display: 'flex', flexDirection: 'column', minWidth: '0' } }, [
          header,
          h('div', {
            style: { flex: '1', padding: '16px', overflow: 'auto', background: BG }
          }, [page])
        ])
      ]);
    }
  };
})();
