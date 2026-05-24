FROM node:24-alpine AS build
WORKDIR /app

COPY DBA_GameAdmin/package*.json ./
RUN npm ci

COPY DBA_GameAdmin/ ./
RUN npm run build

FROM nginx:1.27-alpine AS final
COPY DBA_GameAdmin/nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /app/dist/dba-game-admin/browser /usr/share/nginx/html
EXPOSE 80
