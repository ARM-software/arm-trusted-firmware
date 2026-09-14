# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import html
import re
from dataclasses import dataclass
from urllib.request import Request, urlopen

from bs4 import BeautifulSoup

PAGE_URL = "https://support.arm.com/architectures/a-profile-architecture#Downloads"
USER_AGENT = "sysreg-xml-link-lister/1.2 (+https://support.arm.com/)"
PACKAGE_RE = re.compile(
    r"SysReg_xml_A_profile"
    r"-(?P<year>20\d{2})-(?P<month>0[1-9]|1[0-2])"
    r"(?:_(?P<variant>[A-Za-z0-9_-]+))?\.tar\.gz$",
    re.IGNORECASE,
)
UNICODE_ESCAPE_RE = re.compile(re.escape(chr(92)) + r"u([0-9a-fA-F]{4})")


@dataclass(frozen=True)
class Release:
    url: str
    version: str
    architecture_version: str
    architecture_status: str


class ArchitectureArchiveExtractor:
    def __init__(self, url: str):
        self._releases = self._releases_from_html(self._fetch_page(url))

    def get_latest_release(self):
        latest: Release = self._releases[0]

        for release in self._releases:
            if (
                release.architecture_status == "EAC"
                and release.version >= latest.version
            ):
                latest = release

        return latest

    def _releases_from_html(self, raw_page: str) -> list[Release]:
        """Extract packages directly from their complete download-table rows."""
        found: list[Release] = []
        for document in self._page_documents(raw_page):
            for row in document.find_all("tr"):
                cells = [
                    " ".join(cell.get_text(" ", strip=True).split()) or None
                    for cell in row.find_all(["th", "td"], recursive=False)
                ]

                if len(cells) < 4:
                    continue

                for anchor in row.find_all("a", href=True):
                    url = self._get_package_url(anchor["href"])
                    if url:
                        found.append(Release(
                                url=url,
                                architecture_version=cells[1],
                                architecture_status=cells[2],
                                version=cells[3],
                            )
                        )
        return found

    def _fetch_page(self, url: str) -> str:
        """Fetch the Arm page with a browser-like User-Agent."""
        request = Request(url, headers={"User-Agent": USER_AGENT})
        with urlopen(request, timeout=30) as response:
            charset = response.headers.get_content_charset() or "utf-8"
            return response.read().decode(charset, errors="replace")

    def _get_package_url(self, href: str) -> str | None:
        """Return a non-FAT SysReg XML package URL, or None for another link."""
        url = html.unescape(href).strip().strip(chr(92) + chr(34))
        return url if PACKAGE_RE.search(url) else None

    def _normalise_fragment(self, fragment: str) -> str:
        """Decode HTML and Sitecore JSON unicode escapes in an HTML fragment."""
        return UNICODE_ESCAPE_RE.sub(
            lambda match: chr(int(match[1], 16)), html.unescape(fragment)
        )

    def _page_documents(self, raw_page: str) -> list[BeautifulSoup]:
        """Parse normal markup and HTML fragments embedded in JSON script elements."""
        page = BeautifulSoup(raw_page, "html.parser")
        documents = [page]
        for script in page.find_all("script", type="application/json"):
            content = script.string or script.get_text()
            if "SysReg_xml_A_profile" in content:
                documents.append(BeautifulSoup(self._normalise_fragment(content), "html.parser"))

        return documents
