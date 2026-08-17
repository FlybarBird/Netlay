-- GL.iNet rejects RPC strings that are not [%w. _:#/-].
-- ALSA devices are hw:0,0 (comma) and group passwords can be anything.
return {
  status = true,
  levels = true,
  devices = true,
  save = true,
  connect = true,
  disconnect = true,
  volume = true,
  support = true,
  install_support = true,
  allow_remote_mix = true
}
