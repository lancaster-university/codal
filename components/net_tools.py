import urllib

def is_url(url):
  try:
    result = urllib.parse.urlparse(url)
    return all([result.scheme, result.netloc])
  except ValueError:
    return False