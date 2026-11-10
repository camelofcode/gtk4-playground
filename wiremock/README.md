# Local Articles API

This directory contains the complete local backend used by the Articles API
example. It runs under WireMock through Docker Compose and publishes the API at
`http://localhost:8090`.

From the repository root:

```sh
docker compose -f wiremock/compose.yaml up --detach
curl http://localhost:8090/api/articles?page=1
docker compose -f wiremock/compose.yaml down
```

The mappings provide two pages of summaries and a detail route for every
article. `data/openapi.yaml` documents the same contract. The image fixture is
served from WireMock's `__files` directory and is intentionally generic,
original artwork suitable for this learning project.
