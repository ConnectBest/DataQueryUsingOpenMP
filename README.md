# DataQueryUsingOpenMP

## Overview

**DataQueryUsingOpenMP** is a C++ benchmarking project that explores the
performance benefits of shared‑memory parallelism using **OpenMP** for
querying large in‑memory datasets.

The project evaluates how multi‑threaded execution impacts query
performance on large-scale tabular data by executing analytical queries
across a dataset exceeding **200 million records**.

This work supports experimental research conducted for **CMPE‑275 --
Enterprise Application Development** at **San José State University**.

### Project Goals

-   Evaluate OpenMP parallel performance for data‑intensive workloads
-   Compare single‑threaded vs multi‑threaded query execution
-   Analyze scalability across different thread counts
-   Measure CPU utilization, runtime, and memory behavior under load

The dataset used for experimentation comes from the **NYC Taxi &
Limousine Commission High Volume For‑Hire Vehicle (HVFHV)** trip record
data.

------------------------------------------------------------------------

# Dataset

## NYC TLC HVFHV Trip Data

This project uses the **High Volume For‑Hire Vehicle (HVFHV)** dataset
published by the **New York City Taxi & Limousine Commission (TLC)**.

The dataset contains ride‑hailing trip records for services such as
**Uber and Lyft**.

Each row represents a completed trip and includes fields such as:

-   trip timestamps
-   pickup and dropoff locations
-   trip distance
-   fare information
-   driver payment
-   tips

Official dataset source:

https://www.nyc.gov/site/tlc/about/tlc-trip-record-data.page

------------------------------------------------------------------------

# Dataset Preparation

The TLC publishes monthly datasets in **Apache Parquet format**.\
For benchmarking purposes, we combine multiple months into a **single
CSV file** to simplify ingestion in C++.

The instructions below recreate the dataset used in the experiments.

------------------------------------------------------------------------

# 1. Create a Working Directory

``` bash
mkdir -p ~/hvfhv_2023_jan_may
cd ~/hvfhv_2023_jan_may
```

------------------------------------------------------------------------

# 2. Download Monthly Parquet Files

Download the first five months of **2023 HVFHV trip data**.

``` bash
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-01.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-02.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-03.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-04.parquet
curl -O https://d37ci6vzurychx.cloudfront.net/trip-data/fhvhv_tripdata_2023-05.parquet
```

Verify the downloads:

``` bash
ls -lh *.parquet
```

Expected output:

    fhvhv_tripdata_2023-01.parquet
    fhvhv_tripdata_2023-02.parquet
    fhvhv_tripdata_2023-03.parquet
    fhvhv_tripdata_2023-04.parquet
    fhvhv_tripdata_2023-05.parquet

------------------------------------------------------------------------

# 3. Install DuckDB

DuckDB is used to combine the Parquet files and export them as CSV.

### macOS

``` bash
brew install duckdb
```

### Linux

``` bash
sudo apt install duckdb
```

Verify installation:

``` bash
duckdb --version
```

------------------------------------------------------------------------

# 4. Verify Dataset Size (Optional)

Confirm the expected dataset size before exporting to CSV.

``` bash
duckdb -c "
SELECT COUNT(*) AS row_count
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet');
"
```

Expected output:

    ~94,000,000 rows

(The exact number may vary slightly depending on dataset revisions.)

------------------------------------------------------------------------

# 5. Combine Parquet Files into a Single CSV

DuckDB supports glob patterns for reading multiple files.

``` bash
duckdb -c "
COPY (
SELECT *
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet')
)
TO 'fhvhv_2023_jan_may.csv'
(FORMAT CSV, HEADER);
"
```

This produces:

    fhvhv_2023_jan_may.csv

------------------------------------------------------------------------

# 6. Verify the CSV Dataset

``` bash
ls -lh fhvhv_2023_jan_may.csv
head -n 5 fhvhv_2023_jan_may.csv
wc -l fhvhv_2023_jan_may.csv
```

Example preview:

    hvfhs_license_num,dispatching_base_num,originating_base_num,...
    HV0003,B03404,B03404,2023-01-01 00:18:06,...

------------------------------------------------------------------------

# 7. Optional: Create a Filtered Dataset

For faster experimentation you may generate a filtered dataset.

Example: retain only trips with **trip_miles ≥ 4**

``` bash
duckdb -c "
COPY (
SELECT *
FROM read_parquet('fhvhv_tripdata_2023-0[1-5].parquet')
WHERE trip_miles >= 4
)
TO 'fhvhv_2023_miles_ge_4.csv'
(FORMAT CSV, HEADER);
"
```

Filtered datasets allow faster development cycles while preserving
realistic data distributions.

------------------------------------------------------------------------

# Why CSV?

Although **Parquet** is more efficient for analytics systems, CSV is
used here because it simplifies:

-   low‑level file parsing
-   benchmarking in C/C++
-   predictable memory loading behavior

This allows the project to focus on **parallel query execution
performance** rather than file format complexity.

------------------------------------------------------------------------

# Dataset Citation

NYC Taxi & Limousine Commission (TLC).\
**TLC Trip Record Data**

https://www.nyc.gov/site/tlc/about/tlc-trip-record-data.page

