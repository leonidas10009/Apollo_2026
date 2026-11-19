/**
 * @brief Parsed Apollo release version and comparison helpers.
 *
 * Apollo extends the upstream version format with an optional incremental
 * component (for example ``v2025.121.0-beta.2``), so this class keeps the
 * upstream ``SunshineVersion`` name while parsing Apollo release tags.
 */
class SunshineVersion {
  /**
   * @brief Construct a version from GitHub release data or a version string.
   *
   * @param {?object} release GitHub release data.
   * @param {?string} version Version string when release data is unavailable.
   */
  constructor(release = null, version = null) {
    if (release) {
      this.release = release;
      this.version = release.tag_name;
      this.versionName = release.name;
      this.versionTag = release.tag_tag;
    } else if (version) {
      this.release = null;
      this.version = version;
      this.versionName = null;
      this.versionTag = null;
    } else {
      throw new Error('Either release or version must be provided');
    }
    this.versionParts = this.parseVersion(this.version);
    this.versionMajor = this.versionParts ? this.versionParts[0] : null;
    this.versionMinor = this.versionParts ? this.versionParts[1] : null;
    this.versionPatch = this.versionParts ? this.versionParts[2] : null;
    this.versionIncremental = this.versionParts ? this.versionParts[3] : null;
  }

  /**
   * @brief Split a version string into numeric components.
   *
   * @param {string} version Version string to parse.
   * @return {?number[]} Numeric version components.
   */
  parseVersion(version) {
    if (!version) {
      return null;
    }
    let v = version;
    if (v.startsWith('v')) {
      v = v.substring(1);
    }

    const [mainVer, incrementalVer] = v.split('-');
    const versionParts = mainVer.split('.').map(i => parseInt(i, 10));
    if (incrementalVer) {
      const [prefix, verStr] = incrementalVer.split('.');
      let incremental = parseInt(verStr, 10);
      if (prefix === 'beta') {
        // we couldn't have 2^16 alpha versions?
        incremental <<= 16;
      }
      versionParts.push(incremental);
    }

    return versionParts;
  }

  /**
   * @brief Compare this version with another Apollo version.
   *
   * @param {SunshineVersion|string} otherVersion Version to compare.
   * @param {boolean} [checkIncremental] Whether to compare the incremental component.
   * @return {boolean} Whether this version is greater.
   */
  isGreater(otherVersion, checkIncremental) {
    let otherVersionParts;
    if (otherVersion instanceof SunshineVersion) {
      otherVersionParts = otherVersion.versionParts;
    } else if (typeof otherVersion === 'string') {
      otherVersionParts = this.parseVersion(otherVersion);
    } else {
      throw new TypeError('Invalid argument: otherVersion must be a SunshineVersion object or a version string');
    }

    if (!this.versionParts || !otherVersionParts) {
      return false;
    }
    for (let i = 0; i < Math.min(checkIncremental && 4 || 3, this.versionParts.length, otherVersionParts.length); i++) {
      if (this.versionParts[i] > otherVersionParts[i]) {
        return true;
      } else if (this.versionParts[i] < otherVersionParts[i]) {
        return false;
      }
    }
    return false;
  }
}

export default SunshineVersion;
