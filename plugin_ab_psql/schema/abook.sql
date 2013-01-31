SET client_encoding = 'UTF8';
SET check_function_bodies = false;
SET client_min_messages = warning;
SET search_path = public, pg_catalog;
SET default_tablespace = '';
SET default_with_oids = false;

DROP DATABASE addressbook;

DROP ROLE hylandzax;

CREATE ROLE hylandzax WITH LOGIN PASSWORD 'hyland' CREATEDB;

CREATE DATABASE addressbook
  WITH OWNER = hylandZax
       ENCODING = 'UTF8'
       TABLESPACE = pg_default
       LC_COLLATE = 'German_Germany.1252'
       LC_CTYPE = 'German_Germany.1252'
       CONNECTION LIMIT = -1;

GRANT ALL PRIVILEGES ON DATABASE addressbook to hylandzax;
       
\c addressbook hylandzax - -;

DROP INDEX contacts_idx_001;
DROP TABLE contacts;

CREATE TABLE contacts (
  guid    uuid NOT NULL,
  fn      varchar(48) NOT NULL,
  n       varchar(48),
  org     varchar(48),
  note    varchar(256),
  picture bytea,
  CONSTRAINT contacts_pkey PRIMARY KEY (guid) 
);
CREATE INDEX contacts_idx_001 ON contacts(fn);

DROP INDEX numbers_idx_001;
DROP INDEX numbers_idx_002;
DROP TABLE numbers;

CREATE TABLE numbers (
  contactref  uuid NOT NULL,
  type        integer NOT NULL,
  canonical   varchar(22) NOT NULL, 
  country     varchar(4),       -- ITU-T E.123/E.164 ( +049, ... )
  area        varchar(10),
  local       varchar(10),
  extension   varchar(10),
  FOREIGN KEY(contactref) REFERENCES contacts(guid) ON DELETE CASCADE
);
CREATE INDEX numbers_idx_001 ON numbers(canonical);
CREATE INDEX numbers_idx_002 ON numbers(country, area, local, extension);

\q
