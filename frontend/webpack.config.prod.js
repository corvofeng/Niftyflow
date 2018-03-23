var path = require('path');
var HtmlWebpackPlugin = require('html-webpack-plugin');
var webpackMajorVersion = require('webpack/package.json').version.split('.')[0];

module.exports = {
    context: __dirname,
    entry: [
        './src/index.js'
    ],

    output: {
        path: path.join(__dirname, 'dist/webpack-' + webpackMajorVersion),
        publicPath: '',
        filename: 'bundle.js'
    },
    module: {
        rules: [
            { test: /\.css$/, loaders:['style-loader', 'css-loader'] },
            {     test: /\.(png|jpg|gif)$/,
                use: [
                    {
                        loader: 'file-loader',
                        options: {}
                    }
                ]  },
            { test: /\.html$/, loader: 'html-loader' }
        ]
    },
    plugins: [
        new HtmlWebpackPlugin({
            template: './src/index.html'
        })
    ],
    mode: 'production'
};
